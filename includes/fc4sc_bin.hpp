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

               Date: 2018-Feb-20
******************************************************************************/

/*!
 \file bin.hpp
 \brief Template functions for bin implementations

 This file contains the template implementation of
 bins and illegal bins
 */

#ifndef FC4SC_BIN_HPP
#define FC4SC_BIN_HPP

#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <memory>  // unique_ptr
#include "fc4sc_base.hpp"
#include <assert.h>

namespace fc4sc
{
// forward declarations
template <typename T> class coverpoint;
template <typename T> class binsof;
template <typename T> class bin;

template <typename T>
static std::vector<interval_t<T>> reunion(const bin<T>& lhs, const bin<T>& rhs);

template <typename T>
static std::vector<interval_t<T>> reunion(const bin<T>& lhs, const std::vector<interval_t<T>>& rhs);

template <typename T>
static std::vector<interval_t<T>> intersection(const bin<T>& lhs, const bin<T>& rhs);

template <typename T>
static std::vector<interval_t<T>> intersection(const bin<T>& lhs, const std::vector<interval_t<T>>& rhs);

/*!
 * \brief Defines an abstract class for bin sample behavior strategy
 * \tparam T Type of values in this bin
 */  
template <typename T> 
class sample_strategy {
 public:
  sample_strategy(std::string n) : name(n) {
  }
  // FIXME virtual ~sample_strategy();
  virtual uint64_t sample(const T &val, std::vector<interval_t<T>>& intervals, uint64_t &hits) = 0;

protected:
  std::string name;

};
  
/*!
 * \brief Define a concrete class for bin sample strategies
 * \tparam T Type of values in this bin
 */
template <typename T>
class bin_sample_strategy : public sample_strategy<T> {
public:
  bin_sample_strategy(std::string n) : sample_strategy<T>(n) {
  }
protected:
    std::vector<interval_t<T>> intervals;
public:

  /*!
   * \brief Samples the given value and increments hit counts
   * \param val Current sampled value
   */  
  virtual uint64_t sample(const T &val, std::vector<interval_t<T>>& intervals, uint64_t &hits) override {
    // Just search for the value in the intervals we have
    for (auto i : intervals)
      if (val >= i.first && val <= i.second)
      {
        hits++;
        return 1;
      }
    return 0;
  }
};

/*!
 * \brief Define a concrete class for wildcard bin sample strategies
 * \tparam T Type of values in this bin
 */
template <typename T>
class wildcard_bin_sample_strategy : public sample_strategy<T> {
public:
  wildcard_bin_sample_strategy(std::string n, T m)  : sample_strategy<T>(n) {
    mask=m;
  }
private:
  T mask;
public:
  /*!
   * \brief After adjusting fo the don't care mask samples the given value and increments hit counts
   * \param val Current sampled value
   */  
  virtual uint64_t sample(const T &val, std::vector<interval_t<T>>& intervals, uint64_t &hits) override {
    T loc_val = val & mask;
    // Just search for the value in the intervals we have
    for (auto i : intervals)
      if (loc_val >= i.first && loc_val <= i.second)
        {
          hits++;
          return 1;
        }
    return 0;
  }
};

/*!
 * \brief Define a concrete class for ignore bin sample strategies
 * \tparam T Type of values in this bin
 */
template <typename T>
class ignore_bin_sample_strategy : public sample_strategy<T> {
public:
  ignore_bin_sample_strategy(std::string n)  : sample_strategy<T>(n) {
  }

  /*!
   * \brief Sample does nothing - no hit or miss 
   */
  virtual uint64_t sample(const T &val, std::vector<interval_t<T>>& intervals, uint64_t &hits) override {
    return 0;
  }
};

/*!
 * \brief Define concrete class for illegal bin sample strategies
 * \tparam T Type of values in this bin
 */
template <typename T>
class illegal_bin_sample_strategy : public sample_strategy<T> {
public:
  illegal_bin_sample_strategy(std::string n)  : sample_strategy<T>(n) {
  }

  /*!
   * \brief Same as bin::sample(const T& val)
   *
   * Samples the inner bin instance, but throws an error if the value is found
   */
  virtual uint64_t sample(const T &val, std::vector<interval_t<T>>& intervals, uint64_t &hits) override {
    for (size_t i = 0; i < intervals.size(); ++i)
      if (val >= intervals[i].first && val <= intervals[i].second) {
        // construct exception to be thrown
        std::stringstream ss; ss << val;
        illegal_bin_sample_exception e;
        e.update_bin_info(this->name, ss.str());
        hits++;
        throw e;
      }
  }
};

  
/*!
 * \brief Defines a class for default bins
 * \tparam T Type of values in this bin
 */
template <class T>
class bin : public bin_base
{
private:
  static_assert(std::is_arithmetic<T>::value, "Type must be numeric!");
  friend binsof<T>;
  friend coverpoint<T>;
  
  
  // These constructors are private, making the only public constructor be
  // the one which receives a name as the first argument. This forces the user
  // to give a name for each instantiated bin.

  /*!
   *  \brief Takes a value and wraps it in a pair
   *  \param value Value associated with the bin
   *  \param args Rest of arguments
   */
  template <typename... Args>
  bin(T value, Args... args) noexcept : bin(args...) {
    intervals.push_back(interval(value, value));
  }

  /*!
   *  \brief Adds a new interval to the bin
   *  \param interval New interval associated with the bin
   *  \param args Rest of arguments
   */
  template <typename... Args>
  bin(interval_t<T> interval, Args... args) noexcept : bin(args...) {
    intervals.push_back(interval);
    if (intervals.back().first > intervals.back().second) {
      std::swap(intervals.back().first, intervals.back().second);
    }
  }

protected:
  /*! Default Constructor */
  bin() = default;

  // This is the context switch for sample behavior
  sample_strategy<T>* sample_context  = nullptr;
  
  // The type of UCIS bin (default/user/ignore/illegal)
  std::string ucis_bin_type;

  // The all important hit count for a bin
  uint64_t hits = 0;
  
  
  /*! Storage for the values. All are converted to intervals */
  std::vector<interval_t<T>> intervals;

  /*! Name of the bin */
  std::string name;

public:

  /* Virtual function used to register this bin inside a coverpoint */
  virtual void add_to_cvp(coverpoint<T> &cvp) const
  {
    cvp.bins.push_back(*this);

  }

  /*!
   *  \brief Takes the bin name
   *  \param bin_name Name of the bin
   *  \param args Rest of arguments
   */
  template<typename ...Args>
  explicit bin(const std::string &bin_name, Args... args) noexcept : bin(args...) {
    static_assert(forbid_type<std::string, Args...>::value, "Bin constructor accepts only 1 name argument!");
    this->name = bin_name;
    this->ucis_bin_type = "user";
    this->sample_context = new bin_sample_strategy<T>("");        
        
  }

  /*!
   *  \brief Takes the bin name
   *  \param bin_name Name of the bin
   *  \param args Rest of arguments
   */
  template<typename ...Args>
  explicit bin(const char *bin_name, Args... args) noexcept : bin(std::string(bin_name), args...) {}

  /*! Default Destructor */
  virtual ~bin() = default;

  /*!
   *  \brief Return the bin name
   */
  std::string get_bin_name() const
  {
    return name;
  }
  
  /*!
   *  \brief Return the bin hit number
   */
  uint64_t get_hitcount() const
  {
    return hits;
  }

  /*!
   *  \brief Return the bin intervals
   */
  std::vector<interval_t<T>> get_bin_intervals() const
  {
    return intervals;
  }

  /*!
   *  \brief Return the bin type
   */
  std::string get_bin_type() const
  {
    return ucis_bin_type;
  }
  
  /*!
   * \brief Samples the given value based on the
   * \ context for this object
   */
  virtual uint64_t sample(const T &val)
  {
    assert(sample_context != nullptr);
    return this->sample_context->sample(val,intervals,this->hits); 
  }

  /*!
   * \brief Finds if the given value is contained in the bin
   * \param val Value to search for
   * \returns true if found, false otherwise
   */
  bool contains(const T &val) const
  {
    for (size_t i = 0; i < intervals.size(); ++i)
      if (intervals[i].first <= val && intervals[i].second >= val)
        return true;

    return false;
  }

  bool is_empty() const
  {
    return intervals.empty();
  }

  /*!
   * \brief Writes the bin in UCIS XML format
   * \param stream Where to print it
   */
  virtual void to_xml(std::ostream &stream) const
  {
    stream << "<coverpointBin name=\"" << fc4sc::global::escape_xml_chars(name) << "\" \n";
    stream << "type=\""
           << this->ucis_bin_type
           << "\" "
           << "alias=\"" << this->get_hitcount() << "\""
           << ">\n";

    // Print each range. Coverpoint writes the header (name etc.)
    for (size_t i = 0; i < this->intervals.size(); ++i)
    {
      stream << "<range \n"
	  // Adding "+" before the "this->intervals[i].first" and "this->intervals[i].second"
	  // operands which promotes them to numeric types. This means that char-based types
	  // will be output (in the UCIS DB) in numeric form.
	  // Char based types are problematic when used with value 0 because they are treated
	  // as NULL terminator and results in malformed XML!
             << "from=\"" << +this->intervals[i].first << "\" \n"
             << "to =\"" << +this->intervals[i].second << "\"\n"
             << ">\n";

      // Print hits for each range
      stream << "<contents "
             << "coverageCount=\"" << this->hits << "\">";
      stream << "</contents>\n";
      stream << "</range>\n\n";
    }

    stream << "</coverpointBin>\n";
  }

  friend std::vector<interval_t<T>> reunion<T>(const bin<T>& lhs, const std::vector<interval_t<T>>& rhs);
  friend std::vector<interval_t<T>> intersection<T>(const bin<T>& lhs, const std::vector<interval_t<T>>& rhs);
  friend std::vector<interval_t<T>> reunion<T>(const bin<T>& lhs, const bin<T>& rhs);
  friend std::vector<interval_t<T>> intersection<T>(const bin<T>& lhs, const bin<T>& rhs);
};

/*!
 * \brief Defines a class for illegal bins
 * \tparam T Type of values in this bin
 */
template <class T>
class illegal_bin final : public bin<T>
{
  static_assert(std::is_arithmetic<T>::value, "Type must be numeric!");
protected:
  /* Virtual function used to register this bin inside a coverpoint */
  virtual void add_to_cvp(coverpoint<T> &cvp) const override
  {
    cvp.illegal_bins.push_back(*this);
  }

  illegal_bin() = delete;
public:
  /*!
   *  \brief Forward to parent constructor
   */
  template <typename... Args>
  explicit illegal_bin(Args... args) : bin<T>::bin(args...) {
    this->ucis_bin_type = "illegal";
    this->sample_context = new illegal_bin_sample_strategy<T>("");        
  }

  virtual ~illegal_bin() = default;

};

template <class T>
class ignore_bin final : public bin<T>
{
  static_assert(std::is_arithmetic<T>::value, "Type must be numeric!");
protected:
  /* Virtual function used to register this bin inside a coverpoint */
  virtual void add_to_cvp(coverpoint<T> &cvp) const override
  {
    cvp.ignore_bins.push_back(*this);
  }

  ignore_bin() = delete;
public:
  /*!
   *  \brief Forward to parent constructor
   */
  template <typename... Args>
  explicit ignore_bin(Args... args) : bin<T>::bin(args...) {
    this->ucis_bin_type = "ignore";
    this->sample_context = new ignore_bin_sample_strategy<T>("");    
  }

  virtual ~ignore_bin() = default;
};

/*!
 * \brief Bins that mask don't care care bits. The input argument is a string
 * of binary values with may include don't care characters X or ?. The input 
 * format resembles the functional coverage wildcard format in the IEEE 1800.
 * FIXME The current implemention is limited to binary bits. 
 */

template <class T> 
class wildcard_bin final : public bin<T>
{
protected:
  uint64_t bin_val;
  uint64_t dont_care_mask;
  
  /* Virtual function used to register this bin inside a coverpoint */
  virtual void add_to_cvp(coverpoint<T> &cvp) const override
  {
    cvp.bins.push_back(*this);
  }

  wildcard_bin() = delete;
  
public:
  /*!
   *  \brief Forward to parent constructor
   */
  template <typename... Args>
  explicit wildcard_bin(std::string wildcard_in,Args... args)  : bin<T>::bin(args...) {
    static_assert(std::is_integral<T>::value, "Type must be integral!");          
    this->ucis_bin_type = "user";
    this->name  = wildcard_in;
    wildcard_in.erase(std::remove(wildcard_in.begin(), wildcard_in.end(), '_'), wildcard_in.end());
    this->dont_care_mask = calc_dont_care_mask(wildcard_in);
    bin_val = calc_bin_val(wildcard_in);
    this->intervals.push_back(interval(bin_val, bin_val));
    this->sample_context = new wildcard_bin_sample_strategy<T>(this->name, this->dont_care_mask);
  }

  virtual ~wildcard_bin() = default;
  
  void wildcard_sample(T data_in, T& data_out) {
    data_out =  this->dont_care_mask & data_in;
  }
  
  int unsigned calc_bin_val(std::string wildcard_in) {
    std::string wildcard;
    wildcard.resize(wildcard_in.length());
    for (unsigned int i=0; i< wildcard_in.length(); ++i) {
      if (wildcard_in[i] == 'x' || wildcard_in[i] == '?')
        wildcard[i] = '0'; // normalize dont-care to zero 
      else if (wildcard_in[i] == '1')
        wildcard[i] = '1';
      else
        wildcard[i] = '0';
    }
    return(std::stol(wildcard,nullptr,2));
  }
  
  int unsigned calc_dont_care_mask(std::string wildcard_in) {
    std::string mask;    
    mask.resize(wildcard_in.length());
    for (unsigned int i=0; i< wildcard_in.length(); ++i) {
      if (wildcard_in[i] == 'x' || wildcard_in[i] == '?')
        mask[i] = '0';
      else
        mask[i] = '1';
    }
    return(std::stol(mask,nullptr,2));
  }
};

/*!
 * \brief Heler class for creating an array of bins
 */

template <class T>
class bin_array final : public bin<T>
{
  static_assert(std::is_arithmetic<T>::value, "Type must be numeric!");
protected:
  /* Virtual function used to register this bin inside a coverpoint */
  virtual void add_to_cvp(coverpoint<T> &cvp) const override
  {
    if (this->sparse) {
      // bin array was defined by using a vector of intervals or values
      // create a new bin for each value/interval and add it to the coverpoint
      std::stringstream ss;

      for (size_t i = 0; i < this->intervals.size(); ++i) {
        ss << this->name << "[" << i << "]";
        cvp.bins.push_back(bin<T>(ss.str(), this->intervals[i]));
        ss.str(std::string()); // clear the stringstream
      }
    }
    else {
      // bin array was defined by using an interval which needs to be split into
      // multiple pieces. The interval is found in the this->intervals[0]

      // FIXME: interval_length is not properly calculated for floating point types
      T interval_length = (this->intervals[0].second - this->intervals[0].first) + 1;

      // The if following condition can trigger comparison warnings.
      // Casting interval_length is not a viable option because if T is float
      // or double, we will cast away the floating point and lose information!
      // Nor is casting the count variable to T either, because we might undercast
      // TODO: find a way to work around this issue
      // NOTE: A potential fix would be implementing a template specialization
      // of the bin_array class for floating point types (float/double).
      if (this->count > interval_length) {
        // This bin array interval cannot be split into pieces. Add a single
        // bin containing the whole interval to the coverpoint. We can simply
        // use this object since it already matches the bin that we need!
        cvp.bins.push_back(*this);
      }
      else {
        std::stringstream ss;
        // This bin array interval must be split into pieces.
        T start = this->intervals[0].first;
        T stop = this->intervals[0].second;
        T interval_len = (interval_length + 1) / this->count;

        for (size_t i = 0; i < this->count; ++i) {
          ss << this->name << "[" << i << "]";
          // the last interval, will contain all the extra elements
          T end = (i == (this->count - 1)) ? stop : start + (interval_len - 1);
          cvp.bins.push_back(bin<T>(ss.str(), interval(start, end)));
          start = start + interval_len;
          ss.str(std::string()); // clear the stringstream
        }
      }
    }
  }

  uint64_t count;
  bool sparse = false;

  bin_array() = delete;

public:
  /*!
   * \brief Constructs an bin_array which will split an interval into multiple
   * equal parts. The number of sub-intervals is specified via the count argument
   */
  explicit bin_array(const std::string &name, uint64_t count, interval_t<T> interval) noexcept :
    bin<T>(name, interval), count(count), sparse(false) {}

  /*!
   * \brief Constructs an bin_array from a vector of intervals where each
   * interval will be nested by a bin
   */
  explicit bin_array(const std::string &name, std::vector<interval_t<T>>&& intvs) noexcept :
    count(intvs.size()), sparse(true)
  {
    this->name = name;
    this->intervals = std::move(intvs);
  }

  /*!
   * \brief Constructs an bin_array from a vector of values where each
   * values will be nested by a bin
   */
  explicit bin_array(const std::string &name, const std::vector<T>& intvs) noexcept :
    count(intvs.size()), sparse(true)
  {
    this->name = name;
    this->intervals.clear();
    this->intervals.reserve(this->count);
    // transform each value in the input vector to an interval
    std::transform(intvs.begin(), intvs.end(),
                   std::back_inserter(this->intervals),
                   [](const T& v) { return fc4sc::interval(v,v); });
  }

  virtual ~bin_array() = default;
};

/*
 * Bin wrapper class used when constructing coverpoint via the COVERPOINT macro.
 * Under the hood, the macro instantiates a coverpoint using std::initializer_list
 * as argument. Because the std::initializer_list is limited to one type only,
 * we cannot directly pass any type of bin we want to the coverpoint. In order to
 * do that, we use this class which offers implicit cast from any type of bin and
 * stores it internally as a dynamically allocated object.
 */
template <class T>
class bin_wrapper final {
private:
  friend class coverpoint<T>;

  std::unique_ptr<bin<T>> bin_h;
  bin<T> *get_bin() const { return bin_h.get(); }

public:
  bin_wrapper(bin_wrapper && r) { bin_h = std::move(r.bin_h); }
  bin_wrapper() = delete;
  bin_wrapper(bin_wrapper &) = delete;
  bin_wrapper& operator=(bin_wrapper &) = delete;
  bin_wrapper& operator=(bin_wrapper &&) = delete;
public:
  ~bin_wrapper() = default;
  // Implicit cast to other bin types.
  bin_wrapper(bin<T>        && r) noexcept : bin_h(new bin<T>        (std::move(r))) {}
  bin_wrapper(bin_array<T>  && r) noexcept : bin_h(new bin_array<T>  (std::move(r))) {}
  bin_wrapper(illegal_bin<T>&& r) noexcept : bin_h(new illegal_bin<T>(std::move(r))) {}
  bin_wrapper(ignore_bin<T> && r) noexcept : bin_h(new ignore_bin<T> (std::move(r))) {}
  bin_wrapper(wildcard_bin<T> && r) noexcept : bin_h(new wildcard_bin<T> (std::move(r))) {}
};

} // namespace fc4sc

#endif /* FC4SC_BIN_HPP */
