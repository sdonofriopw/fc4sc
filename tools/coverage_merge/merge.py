"""
/******************************************************************************

   Copyright 2003-2018 AMIQ Consulting s.r.l.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

******************************************************************************/
/******************************************************************************
   Original Authors: Teodor Vasilache and Dragos Dospinescu,
                     AMIQ Consulting s.r.l. (contributors@amiq.com)

               Date: 2018-Oct-07
******************************************************************************/
"""
import os
import sys
import xml.etree.ElementTree as ET
from fnmatch import fnmatch

# namespace map
nsstr = 'ucis'
ns = {nsstr : 'http://www.w3.org/2001/XMLSchema-instance'}
# NOT USED; TODO: REMOVE?
ucis_db = {
    "instanceCoverages"  : '{0}:instanceCoverages' .format(nsstr),
    "covergroupCoverage" : '{0}:covergroupCoverage'.format(nsstr),
    "cgInstance"         : '{0}:cgInstance'        .format(nsstr),
    "coverpoint"         : '{0}:coverpoint'        .format(nsstr),
    "coverpointBin"      : '{0}:coverpointBin'     .format(nsstr),
    "range"              : '{0}:range'             .format(nsstr),
    "contents"           : '{0}:contents'          .format(nsstr),
    "cross"              : '{0}:cross'             .format(nsstr),
    "crossBin"           : '{0}:crossBin'          .format(nsstr),
    "crossExpr"          : '{0}:crossExpr'         .format(nsstr),
    "index"              : '{0}:index'             .format(nsstr),
    "userAttr"           : '{0}:userAttr'          .format(nsstr)
}

def findall_ucis_children(element, subElementName):
    global ns, nsstr
    return element.findall('{0}:{1}'.format(nsstr, subElementName), ns)
    

def find_xmls(directory):
    for rootdir, _, files in os.walk(directory):
        for fname in files:
            if fnmatch(fname, '*.xml'):
                filename = os.path.join(rootdir, fname)
                yield filename

def parseXML(parseRoot, mergeDBtree):
    mergeDBroot = mergeDBtree.getroot()
    
    """
    Description of the UCIS DB hierarchy generated by the current FC4SC implementation:
    
    UCIS top level element
    |
    -> instanceCoverages [0:n]
       |  moduleName : name of the covergroup type
       |
       -> cgInstance [0:n]
          |  name : name of the covergroup instance
          |
          -> coverpoint [0:n]
          |  |  name : name of the coverpoint
          |  |
          |  -> coverpointBin [0:n]
          |     | name : name of the bin
          |     | type : the type of bin (default/ignore/illegal)
          |     |
          |     -> range [0:n]
          |        | from : start value of the interval
          |        | to   : end value of the interval
          |        |
          |        -> contents 
          |           | coverageCount : the number of hits registered in this interval
          |           0
          | 
          -> cross [0:n]
             | name : name of the cross
             |
             -> crossBin [0:n]
                | name : name of the cross bin
                | 
                -> index
                -> index 
                .
                .     Number of indexes = number of crossed coverpoints
                .
                -> index  
                | 
                -> contents                                                        
                   | coverageCount : the number of hits registered in this cross bin
                   0                                                               
                
    Note that this only contains the elements which are relevant for merging!
    
    Merging steps:
     1) Parse covergroup types: for each "instanceCoverages" element:
            if this element does not exist in the mergeDBtree:
                add this element to the mergeDBtree directly under the root element
            else: goto step 2
        Note: same covergroup type condition: equality of the 'moduleName' attribute 
            
     2) Parse covergroup instances: for each "covergroupCoverage/cgInstance" element:
            if this element does not exist in the "covergroupCoverage" element of mergeDB:
                add this element under the "covergroupCoverage" element 
            else: goto step 3
        Note: same covergroup instance condition: equality of the 'name' attribute
                
     3) Parse coverpoints: for each "coverpoint" element:
            if this element does not exist in the "cgInstance" element of mergeDB:
                raise exception: coverpoint not present in the mergeDB! 
            else: goto step 4
        Note: same coverpoint condition: equality of the 'name' attribute
        
     4) Parse bins: for each "bin" element:
            if this element does not exist in the "coverpoint" element of mergeDB:
                add this element under the "coverpoint" element 
            else: goto step 5
        Note: same bin condition: equality of the 'name' attribute
        
     5) Sum the bin ranges' hit counts: for each "range" element:
            if this element does not exist in the "bin" element:
                raise exception: bin is different than expected!
            else:
                add to the coverageCount
        Note: same range condition: equality of the 'name' attribute
        Note2: if 2 bins have the same name, but of different from, to or type attribute values => error
    
     6) Parse crosses: for each "cross" element:
            if this element does not exist in the "cgInstance" element of mergeDB:
                raise exception: cross not present in the mergeDB! 
            else: goto step 7
        Note: same cross condition: equality of the 'name' attribute
        
     7) Parse crosses bins: for each "crossBin" element:
            if this element does not exist in the "cross" element of mergeDB:
                add this element under the "cross" element 
            else: goto step 8
        Note: same crossBin condition: the list of index elements have the same value, in the
        same order
            
    """
    global ns, nsstr
    """ 1) Parse covergroup types """
    for instanceCoverages in findall_ucis_children(parseRoot, "instanceCoverages"):
        covergroupTypeNameAttrib = 'moduleName'
        covergroupTypeName = instanceCoverages.get(covergroupTypeNameAttrib)
        xpath_query = "./*[@{1}='{2}']/{0}:covergroupCoverage".format(
            nsstr, covergroupTypeNameAttrib, covergroupTypeName)
        searchElement = parseRoot.find(xpath_query, ns)

        print("Parsing covergroup type: {0}".format(covergroupTypeName))
        if searchElement is not None:
            parse_covergroup_type(searchElement, xpath_query, mergeDBtree)
            print("\n")
        else:
            print("Found new coverage type [{0}]".format(covergroupTypeName))
            mergeParent = mergeDBroot
            mergeParent.append(instanceCoverages) # add the element to the mergedDB under root element

def parse_covergroup_type(covergroup_type, parent_query, mergeDBtree):
    global ns
    """ Parse covergroup instance """
    for cgInstance in findall_ucis_children(covergroup_type, "cgInstance"):
        cgInstNameAttrib = 'name'
        cgInstName = cgInstance.get(cgInstNameAttrib)
        xpath_query = parent_query + "/" + "*[@{0}='{1}']".format(cgInstNameAttrib, cgInstName)
        searchElement = mergeDBtree.find(xpath_query, ns)
        
        print ("\t[cgInstance] {0}".format(cgInstName))
        if searchElement is not None:
            parse_coverpoints(cgInstance, xpath_query, mergeDBtree)
            parse_crosses(cgInstance, xpath_query, mergeDBtree)
        else:
            print("Found new coverage instance [{0}]".format(cgInstName))
            mergeParent = mergeDBtree.find(parent_query, ns)
            mergeParent.append(cgInstance) # add the element to the covergroup

def parse_coverpoints(cgInstance, parent_query, mergeDBtree):
    global ns
    """ Parse coverpoint """
    for coverpoint in findall_ucis_children(cgInstance, "coverpoint"):
        cvpNameAttrib = 'name'
        cvpName = coverpoint.get(cvpNameAttrib)
        xpath_query = parent_query + "/" + "*[@{0}='{1}']".format(cvpNameAttrib, cvpName)
        searchElement = mergeDBtree.find(xpath_query, ns)
    
        print ("\t\t[coverpoint] {0}".format(cvpName))
        if searchElement is not None:
            parse_coverpoint_bins(coverpoint, xpath_query, mergeDBtree)
        else:
            raise ValueError("coverpoint not present in the mergeDB!")
        
def parse_coverpoint_bins(coverpoint, parent_query, mergeDBtree):
    """ Parse bins """
    for bin in findall_ucis_children(coverpoint, "coverpointBin"):
        binNameAttrib = 'name'
        binName = bin.get(binNameAttrib)
        xpath_query = parent_query + "/" + "*[@{0}='{1}']".format(binNameAttrib, binName)
        binMergeElement = mergeDBtree.find(xpath_query, ns)
        
        if binMergeElement is not None:
            merge_bin_hits(bin, binMergeElement, xpath_query, mergeDBtree)
        else:
            print("\t\tFound new bin [{0}]".format(binName))
            mergeParent = mergeDBtree.find(xpath_query + "/..", ns)
            mergeParent.append(bin) # add the bin to the covergpoint
        
def merge_bin_hits(bin, binMergeElement, parent_query, mergeDBtree):
    global ns, nsstr
    """ Sum the bin ranges' hit counts """
    # merge hits for bins which are present in both the parsed DB and mergeDB
    for range in findall_ucis_children(bin, "range"):
        contents = range.find('{0}:contents'.format(nsstr), ns)
        rangeHitCount = int(contents.get('coverageCount'))
        xpath_query = parent_query + "/" + '{0}:range'.format(nsstr)
        searchElement = mergeDBtree.find(xpath_query, ns)
        
        if searchElement is None:
            raise ValueError("Range not found! Bin contents differ between mergeDB and parsed XML!")
        
        sameFrom = searchElement.get('from') == range.get('from')
        sameTo = searchElement.get('to') == range.get('to')
        
        if not (sameFrom and sameTo):
            raise ValueError("Range limits differ between mergeDB and parsed XML!")
        
        mergeContentsElement = searchElement.find('{0}:contents'.format(nsstr), ns)
        parsedContentsElement = range.find('{0}:contents'.format(nsstr), ns)
        totalhits = int(mergeContentsElement.get('coverageCount'))
        parsedHits = int(parsedContentsElement.get('coverageCount'))
        totalhits += parsedHits
        
        # NOTE: alias attribute is set in the coverpointBin element because the
        # javascript gui application uses this field for showing the number of hits! 
        binMergeElement.set('alias', str(totalhits))
        mergeContentsElement.set('coverageCount', str(totalhits))

    print ("\t\t\t[bin:{1}] {0} -> {2}".format(
        bin.get('name'), bin.get('type'), totalhits))    
    
def parse_crosses(cgInstance, parent_query, mergeDBtree):
    global ns, nsstr
    for cross in findall_ucis_children(cgInstance, "cross"):
        crossNameAttrib = 'name'
        crossName = cross.get(crossNameAttrib)
        xpath_query = parent_query + "/" + "*[@{0}='{1}']".format(crossNameAttrib, crossName)
        mergeCrossElement = mergeDBtree.find(xpath_query, ns)
        
        print ("\t\t[cross] {0}".format(crossName))
        if mergeCrossElement is None:
            raise ValueError("cross not present in the mergeDB!")
            continue # skip processing the sub-elements
        
        # skip processing crosses with no hits in the parse XML
        if cross.find('{0}:crossBin'.format(nsstr), ns) is None:
            print("\t\t\tParsed cross is empty; skipping...")
            continue
         
        # the number of coverpoints crossed by this element
        numCvps = len(findall_ucis_children(mergeCrossElement, 'crossExpr'))
        
        """ Parse cross bins """
        mergeMap = {}
        
        # parse the mergeDB and store all existing cross bins and their associated hit count
        # then, parse the current XML and update the map with the new information
        # then, remove all the the crossBin elements from the cross
        # then, create new crossBins elements matching the information stored in the map!
        for crossBin in findall_ucis_children(mergeCrossElement, 'crossBin'):
            binIndexes = []
            for index in findall_ucis_children(crossBin, 'index'):
                binIndexes.append(int(index.text))
            
            contentsElement = crossBin.find('{0}:contents'.format(nsstr), ns)
            hitCount = int(contentsElement.get('coverageCount'))
            
            if len(binIndexes) != numCvps:
                raise ValueError("Found crossBin of bigger size than the number of coverpoints!") 
            
            tupleIndexes = tuple(binIndexes)
            mergeMap[tupleIndexes] = hitCount
            # remove crossBin
            mergeCrossElement.remove(crossBin)
        
        for crossBin in findall_ucis_children(cross, 'crossBin'):
            binIndexes = []
            for index in findall_ucis_children(crossBin ,'index'):
                binIndexes.append(int(index.text))
            
            contentsElement = crossBin.find('{0}:contents'.format(nsstr), ns)
            hitCount = int(contentsElement.get('coverageCount'))
            
            tupleIndexes = tuple(binIndexes)
            if tupleIndexes in mergeMap:
                mergeMap[tupleIndexes] = mergeMap[tupleIndexes] + hitCount
            else:
                mergeMap[tupleIndexes] = hitCount
        
        crossBinString = """<{0}:crossBin name="" key="0" type="default" xmlns:{0}="{1}">\n"""
        for _ in range(numCvps):
            crossBinString += "<{0}:index>0</{0}:index>\n"
            
        crossBinString += """<{0}:contents coverageCount="0"></{0}:contents>\n"""
        crossBinString += "</{0}:crossBin>\n"
        crossBinString = crossBinString.format(nsstr, ns[nsstr])
             
        # update crossBins element and append it to the mergeCrossElement
        for indexesTuple in mergeMap:
            # create new crossBin element to be added to the cross
            crossBinElement = ET.fromstring(crossBinString)
            print("\t\t\t" + str(indexesTuple) + " -> " + str(mergeMap[indexesTuple]))
            
            # get a generator for the index elements contained by this crossBin;
            # we will need to manually iterate through this generator when updating the indexes
            indexElementGen = iter(findall_ucis_children(crossBinElement, 'index'))
            for i in range(len(indexesTuple)):
                # update index element value
                indexElementValue = indexesTuple[i]
                indexElement = next(indexElementGen)
                indexElement.text = str(indexElementValue)
                
            # update the contents element with the merged data
            contentsElement = crossBinElement.find('{0}:contents'.format(nsstr), ns)
            contentsElement.set('coverageCount', str(mergeMap[indexesTuple]))
            # add the contents element to the cross in the mergeDB
            mergeCrossElement.append(crossBinElement)
            
        global ucis_db
        # move the user attribute element to the end of the cross
        userAttrElement = mergeCrossElement.find(ucis_db['userAttr'], ns)
        mergeCrossElement.remove(userAttrElement)
        mergeCrossElement.append(userAttrElement)
        

if __name__ == "__main__":
    # the search top directory is by default the execution directory 
    search_top_dir = os.getcwd()
    merged_db_name = "coverage_merged_db.xml"
    if len(sys.argv) > 1: # if specified file path
        search_top_dir = sys.argv[1]
    if len(sys.argv) > 2: # if specified merged database name
        merged_db_name = sys.argv[2]

    print("Searching for XMLs in top directory: \n{0}\n".format(search_top_dir))
    merged_db_path = os.path.join(search_top_dir, merged_db_name)
    
    # register the UCIS namespace
    ET.register_namespace(nsstr, 'http://www.w3.org/2001/XMLSchema-instance')
    
    # TODO: update exceptions to be more verbose in function parseXML
    # Needed information:
    # Context: full path to element which produces error on parsing
    # Info: error description
    # Source XML files where the element(s) is/are found  
    
    # the master ucis DB which will be "merged" into when parsing additional DBs
    mergeDB = None
    # list of the file names that are successfully parsed and merged
    filelist = []
    for filename in find_xmls(search_top_dir):
        # found file matches the output file; skip it
        if filename == merged_db_path:
            print("Warning! Input File: \n{0}\nmatches output target file => will not be parsed!".format(filename))
            continue
        
        print("Found XML file: {0}".format(filename))
        
        # parse the found XML  
        parseTree = ET.parse(filename)
        parseRoot = parseTree.getroot()
        
        tagstr = parseRoot.tag[-len("UCIS"):]
        if tagstr != "UCIS":
            print("Skipping non-UCIS DB XML file with tag [{0}]".format(tagstr))
            continue

        filelist.append(filename)
        if mergeDB is None:
            print("First UCIS XML found; setting as base DB!")
            mergeDB = parseTree
        else:
            # TODO: surround by try-catch and handle thrown exceptions
            parseXML(parseRoot, mergeDB)
        
    
    # TODO: parse the resulted DB and change the "UCIS ID" attributes to be unique
    # can use an ElementTree tree walker for this task!
    
    """
    FIXME: Known problems
    1) Some Coverpoint bins can be present in one covergroup instance, but not in another.
    In other words, different covergroup instances of the the same type can have bins which 
    might not be present in a certain coverpoint. This creates a problem when merging crosses, 
    as the current implementation does NOT account for this case!
    
    2) Top level UCIS element attributes of the resulted merged DB have to be updated!
    ====================
    parentId="200" 
    logicalName="string" 
    physicalName="string" 
    kind="string" 
    testStatus="true" 
    simtime="1.051732E7" 
    timeunit="string" 
    runCwd="string" 
    cpuTime="1.051732E7" 
    seed="string" 
    cmd="string" 
    args="string" 
    compulsory="string" 
    date="2004-02-14T19:44:14" 
    userName="string" 
    cost="1000.00" 
    toolCategory="string" 
    ucisVersion="string" 
    vendorId="string" 
    vendorTool="string" 
    vendorToolVersion="string" 
    sameTests="42" 
    comment="string"
    ====================
    """
    
    if not filelist:
        print("Error! No XML files found under " + search_top_dir)
        exit(1)
    
    mergeDB.write(file_or_filename = merged_db_path,
                  encoding = "UTF-8", 
                  xml_declaration = True)

    print("Done!");
    print("Searching was done recursively under directory: \n{0}\n".format(search_top_dir))
    
    print("List of merged UCIS DB files:")
    for f in filelist:
        print(f)
        
    print("\nResulted merged UCIS DB can be found at:\n" + merged_db_path)