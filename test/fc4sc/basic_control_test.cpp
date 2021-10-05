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

#include "fc4sc.hpp"
#include "gtest/gtest.h"
class cvg_control_test : public covergroup {
public:

  CG_CONS(cvg_control_test) {/*setup_coverage();*/};
  int sample_point1;
  int sample_point2;
  int sample_point3;
  int sample_point4;
  int sample_point5;
  int sample_point6;
  int sample_point7;
  int sample_point8;
  
  
  void sample(const int& x) {
    this->sample_point1 = x;
    this->sample_point2 = x;
    this->sample_point3 = x;

    this->sample_point4 = x;
    this->sample_point5 = x;
    this->sample_point6 = x;


    this->sample_point7 = x;
    this->sample_point8 = x;
    
    covergroup::sample();
  }

  
  COVERPOINT(int, cvp1, sample_point1) { fc4sc::wildcard_bin<int>( "xxx_1__" ) };
  COVERPOINT(int, cvp2, sample_point2) { fc4sc::wildcard_bin<int>( "xx_1_x_" ) };
  COVERPOINT(int, cvp3, sample_point3) { fc4sc::wildcard_bin<int>( "_x1xx" ) };
  COVERPOINT(int, cvp4, sample_point4) { fc4sc::wildcard_bin<int>( "xx_x0" ) };  
  COVERPOINT(int, cvp5, sample_point5) { fc4sc::wildcard_bin<int>( "xx0x_" ) };    
  COVERPOINT(int, cvp6, sample_point6) { fc4sc::wildcard_bin<int>( "x0x_x" ) };    
  COVERPOINT(int, cvp7, sample_point7) {
    {
      fc4sc::wildcard_bin<int>( build_1010_string() )
    }
  };      
  coverpoint<int> cvp8  = covergroup::cg_register_cvp<int>(&cvp8, "cvp8",
                                                           [this]() -> int { return (sample_point8); },
                                                           "sample_point8",
                                                           [this]() -> bool { return (true); },
                                                           ""
                                                           ) = {
    fc4sc::wildcard_bin<int>( "0101" )
  };

  std::string build_1010_string() {
    std::string str;
    str.resize(5);
    for (int i=0;i<5;i++) {
      if (i == 0)
        str[i] = '_';
      else if (i%2)
        str[i] = '1';
      else
        str[i] = '0';
    }
    std::cout << "string is "<< str  << '\n';    
    return(str);
  };

};


TEST(api, control) {

  cvg_control_test cvg;

  
  EXPECT_EQ(cvg.get_inst_coverage(), 0);
  /*  
  cvg.stop();
  cvg.sample(1);

  EXPECT_EQ(cvg.get_inst_coverage(), 0);
  */
  cvg.start();
  //cvg.cvp1.stop();
  /*  cvg.sample(1);
  cvg.sample(2);
  cvg.sample(4);
  */
  //cvg.sample(0x3);
  //cvg.sample(0x7);

  cvg.sample(3);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp3.get_inst_coverage(), 0);  
  
  EXPECT_EQ(cvg.cvp4.get_inst_coverage(), 0);
  EXPECT_EQ(cvg.cvp5.get_inst_coverage(), 0);
  EXPECT_EQ(cvg.cvp6.get_inst_coverage(), 100);  


  EXPECT_EQ(cvg.cvp7.get_inst_coverage(), 0);  

  EXPECT_EQ(cvg.cvp8.get_inst_coverage(), 0);  
  
  fc4sc::global::coverage_save("basic_control_sample_1_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");

  cvg.sample(2);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp3.get_inst_coverage(), 0);  
  
  EXPECT_EQ(cvg.cvp4.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp5.get_inst_coverage(), 0);
  EXPECT_EQ(cvg.cvp6.get_inst_coverage(), 100);  

  EXPECT_EQ(cvg.cvp7.get_inst_coverage(), 0);  

  EXPECT_EQ(cvg.cvp8.get_inst_coverage(), 0);  

  fc4sc::global::coverage_save("basic_control_sample_2_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");

  //1010
  cvg.sample(0xa);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp3.get_inst_coverage(), 0);  

  EXPECT_EQ(cvg.cvp4.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp5.get_inst_coverage(), 0);
  EXPECT_EQ(cvg.cvp6.get_inst_coverage(), 100);  
  
  EXPECT_EQ(cvg.cvp7.get_inst_coverage(), 100);  

  EXPECT_EQ(cvg.cvp8.get_inst_coverage(), 0);  
  
  fc4sc::global::coverage_save("basic_control_sample_3_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");
  
  cvg.sample(0x5);

  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp3.get_inst_coverage(), 100);  

  EXPECT_EQ(cvg.cvp4.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp5.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp6.get_inst_coverage(), 100);  

  EXPECT_EQ(cvg.cvp7.get_inst_coverage(), 100);  

  EXPECT_EQ(cvg.cvp8.get_inst_coverage(), 100);  
  
  fc4sc::global::coverage_save("basic_control_sample_4_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");
  
}
