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

class wildcard_cov {
private:
  std::string wildcard_name;
  int unsigned mask_int;
  int unsigned wildcard_int;
public:  
  wildcard_cov() {
  }
  wildcard_cov(std::string wildcard_in) {
    wildcard_name = wildcard_in;
    mask_int = calc_mask(wildcard_in);
    wildcard_int = calc_wildcard(wildcard_in);
    printf("wildcard_int for %s is %x mask is 0x%x\n", get_name().c_str(), wildcard_int, mask_int); 
  }

  std::string get_name() {
    return(wildcard_name);
  }

  int unsigned calc_wildcard(std::string wildcard_in) {
    std::string wildcard;
    wildcard.resize(wildcard_in.length());
    for (unsigned int i=0; i< wildcard_in.length(); ++i) {
      if (wildcard_in[i] == 'x' || wildcard_in[i] == '?')
        wildcard[i] = '0';
      else if (wildcard_in[i] == '1')
        wildcard[i] = '1';
      else
        wildcard[i] = '0';
    }
    return(std::stoi(wildcard,nullptr,2));
  }
  
  int unsigned calc_mask(std::string wildcard_in) {
    std::string mask;    
    mask.resize(wildcard_in.length());
    for (unsigned int i=0; i< wildcard_in.length(); ++i) {
      if (wildcard_in[i] == 'x' || wildcard_in[i] == '?')
        mask[i] = '0';
      else
        mask[i] = '1';
    }
    return(std::stoi(mask,nullptr,2));
  }
  
  int unsigned sample(int unsigned data_in) {
    int unsigned sample_out =  mask_int & data_in;
    printf("wildcard sample for %s data_in %x wildcard %x mask is 0x%x sample out 0x%x\n",
           get_name().c_str(),
           data_in,
           wildcard_int,
           mask_int,
           sample_out); 
    
    return(sample_out);
  }
  
  int unsigned get_coverage_bin() {
    return (wildcard_int);
  }

};


class cvg_control_test : public covergroup {
public:

  CG_CONS(cvg_control_test) {};

  wildcard_cov cov_xxx1{"xxx1"};
  wildcard_cov cov_xx1x{"xx1x"};
  wildcard_cov cov_x1xx{"x1xx"};

  wildcard_cov cov_xxx0{"xxx0"};
  wildcard_cov cov_xx0x{"xx0x"};
  wildcard_cov cov_x0xx{"x0xx"};

  wildcard_cov cov_1010{"1010"};

  wildcard_cov cov_0101{"0101"};

  int SAMPLE_POINT(sample_point1, cvp1);
  int SAMPLE_POINT(sample_point2, cvp2);
  int SAMPLE_POINT(sample_point3, cvp3);

  int SAMPLE_POINT(sample_point4, cvp4);
  int SAMPLE_POINT(sample_point5, cvp5);
  int SAMPLE_POINT(sample_point6, cvp6);

  int SAMPLE_POINT(sample_point7, cvp7);

  int SAMPLE_POINT(sample_point8, cvp8);

  void sample(const int& x) {
    this->sample_point1 = cov_xxx1.sample(x);
    this->sample_point2 = cov_xx1x.sample(x);
    this->sample_point3 = cov_x1xx.sample(x);

    this->sample_point4 = cov_xxx0.sample(x);
    this->sample_point5 = cov_xx0x.sample(x);
    this->sample_point6 = cov_x0xx.sample(x);


    this->sample_point7 = cov_1010.sample(x);
    this->sample_point8 = cov_0101.sample(x);
    
    covergroup::sample();
  }

  coverpoint<int> cvp1 = coverpoint<int> (this, bin<int>(cov_xxx1.get_name(), cov_xxx1.get_coverage_bin()));
  coverpoint<int> cvp2 = coverpoint<int> (this, bin<int>(cov_xx1x.get_name(), cov_xx1x.get_coverage_bin()));
  coverpoint<int> cvp3 = coverpoint<int> (this, bin<int>(cov_x1xx.get_name(), cov_x1xx.get_coverage_bin()));

  coverpoint<int> cvp4 = coverpoint<int> (this, bin<int>(cov_xxx0.get_name(), cov_xxx0.get_coverage_bin()));
  coverpoint<int> cvp5 = coverpoint<int> (this, bin<int>(cov_xx0x.get_name(), cov_xx0x.get_coverage_bin()));
  coverpoint<int> cvp6 = coverpoint<int> (this, bin<int>(cov_x0xx.get_name(), cov_x0xx.get_coverage_bin()));
  
  coverpoint<int> cvp7 = coverpoint<int> (this, bin<int>(cov_1010.get_name(), cov_1010.get_coverage_bin()));
  
  coverpoint<int> cvp8 = coverpoint<int> (this, bin<int>(cov_0101.get_name(), cov_0101.get_coverage_bin()));

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
