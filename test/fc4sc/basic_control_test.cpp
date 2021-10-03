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

  

  int SAMPLE_POINT(sample_point1, cvp1);
  int SAMPLE_POINT(sample_point2, cvp2);
  int SAMPLE_POINT(sample_point3, cvp3);

  int SAMPLE_POINT(sample_point4, cvp4);
  int SAMPLE_POINT(sample_point5, cvp5);
  int SAMPLE_POINT(sample_point6, cvp6);

  int SAMPLE_POINT(sample_point7, cvp7);

  //int SAMPLE_POINT(sample_point8, cvp8);

  /*
  int sample_pointa;
  int sample_pointb;
  int sample_pointc;
  int sample_pointd;
  */
  int sample_point8;
  
  //  int SAMPLE_POINT(sample_pointb, cvp_wildcard[1]*);
  // int SAMPLE_POINT(sample_pointc, cvp_wildcard[2]*);
  // int SAMPLE_POINT(sample_pointd, cvp_wildcard[3]*);
  
  void sample(const int& x) {
    this->sample_point1 = x;
    this->sample_point2 = x;
    this->sample_point3 = x;

    this->sample_point4 = x;
    this->sample_point5 = x;
    this->sample_point6 = x;


    this->sample_point7 = x;
    this->sample_point8 = x;

    /*
    this->sample_pointa = x;
    this->sample_pointb = x;
    this->sample_pointc = x;
    this->sample_pointd = x;
    */
    
    covergroup::sample();
  }

  wildcard_cov cov_xxx1{"xxx_1__"};  
  wildcard_cov cov_xx1x{"xx_1_x_"};
  wildcard_cov cov_x1xx{"_x1xx"};

  wildcard_cov cov_xxx0{"xx_x0"};
  wildcard_cov cov_xx0x{"xx0x_"};
  wildcard_cov cov_x0xx{"x0x_x"};

  wildcard_cov cov_1010{"1010_"};

  wildcard_cov cov_0101{"0101"};

  coverpoint<int> cvp1 = coverpoint<int> (this, &cov_xxx1);
  coverpoint<int> cvp2 = coverpoint<int> (this, &cov_xx1x);
  coverpoint<int> cvp3 = coverpoint<int> (this, &cov_x1xx);
  
  coverpoint<int> cvp4 = coverpoint<int> (this, &cov_xxx0);
  coverpoint<int> cvp5 = coverpoint<int> (this, &cov_xx0x);
  coverpoint<int> cvp6 = coverpoint<int> (this, &cov_x0xx);
  
  coverpoint<int> cvp7 = coverpoint<int> (this, &cov_1010);
  
  //  coverpoint<int> cvp8 = coverpoint<int> (this, &cov_0101);
  coverpoint<int> cvp8  = covergroup::cg_register_cvp<int>(&cvp8, "cvp8",
                                                           [this]() -> int { return (sample_point8); },
                                                           "sample_point8",
                                                           [this]() -> bool { return (true); },
                                                           ""
                                                           ) = {
    wildcard_cov(cov_0101)
    //&cov_0101
    //cov_0101
    //        bin< int >( "low1" , interval(1,6), 7)
    
  };

   /*
     COVERPOINT(int, cvp8, sample_point8) {
    bin< int >( "low1" , interval(1,6), 7)

};
   */
  
  /*
  wildcard_cov *cov_wildcard[4];
  coverpoint<int> *cvp_wildcard[4];
  coverpoint<int> cvp_0 = coverpoint<int> (this, cov_wildcard[0]);
  coverpoint<int> cvp_1 = coverpoint<int> (this, cov_wildcard[1]);
  coverpoint<int> cvp_2 = coverpoint<int> (this, cov_wildcard[2]);
  coverpoint<int> cvp_3 = coverpoint<int> (this, cov_wildcard[3]);
  void setup_coverage(){


    sample_pointa = static_cast<decltype(sample_pointa)>(covergroup::set_strings(cvp_wildcard[0], &sample_pointa, "cvp_wildcard[0]", "sample_pointa"));
    sample_pointb = static_cast<decltype(sample_pointb)>(covergroup::set_strings(cvp_wildcard[1], &sample_pointb, "cvp_wildcard[1]", "sample_pointb"));
    sample_pointc = static_cast<decltype(sample_pointc)>(covergroup::set_strings(cvp_wildcard[2], &sample_pointc, "cvp_wildcard[2]", "sample_pointc"));
    sample_pointd = static_cast<decltype(sample_pointd)>(covergroup::set_strings(cvp_wildcard[3], &sample_pointd, "cvp_wildcard[3]", "sample_pointd"));


    for(int unsigned i=0; i<4; i++ ) {
      std::string temp = "xxxx";
      std::vector<char> myVector( temp.begin(), temp.end() );
      for( int unsigned j = 0; j < temp.size(); j++ ) {
        if (j == i) {
          temp[j] = '1';
        }
      }
      printf("string temp is %s\n", temp.c_str());
      cov_wildcard[i] = new wildcard_cov(temp);
      cvp_wildcard[i] = new coverpoint<int> (this, cov_wildcard[i]);
      //coverpoint<int> cvp[i] = coverpoint<int> (this, cov_wildcard[i]);
      //coverpoint<int> cvp = coverpoint<int> (this, cov_wildcard[1])        
    }

  }
  */

  
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
  
  
  //  EXPECT_EQ(cvg.get_inst_coverage(), 66.6667);
  /*
  cvg.cvp1.start();
   cvg.sample(1);
  
  //cvg.sample(0x3);
  cvg.sample(0x7);

    
  EXPECT_EQ(cvg.cvp1.get_inst_coverage(), 100);
  EXPECT_EQ(cvg.cvp2.get_inst_coverage(), 100);  

  cvg.sample(0x7);

    EXPECT_EQ(cvg.get_inst_coverage(), 100);

  fc4sc::global::coverage_save("basic_control_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");
  */
}
