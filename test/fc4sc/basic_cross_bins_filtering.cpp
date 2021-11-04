#include "fc4sc.hpp"
#include "gtest/gtest.h"

class basic_cross_bins_filtering_test: public covergroup {
public:
  CG_CONS(basic_cross_bins_filtering_test){
    cvp1.option.weight = 0;
    cvp2.option.weight = 0;
  };

  int sample_point_1;
  int sample_point_2;
  int sample_point_3;

  void sample(const int& x, const int& y,const int& z) {
    this->sample_point_1 = x;
    this->sample_point_2 = y;
    this->sample_point_3 = z;
    covergroup::sample();
  }
  COVERPOINT(int, cvp1, sample_point_1 ) {
    bin<int>("cvp1_one", 1),
    bin<int>("cvp1_two", 2)
   };

  COVERPOINT(int, cvp2, sample_point_2 ) {
    bin<int>("cvp2_one", 1),
    bin<int>("cvp2_two", 2),
    bin<int>("cvp2_three", 3),
    bin<int>("cvp2_four", 4),
    ignore_bin<int>("cvp2_five", 5)
   };  
  
  COVERPOINT(int, cvp3, sample_point_3 ) {
    bin<int>("cvp3_one", 1),
    bin<int>("cvp3_two", 2),
    bin<int>("cvp3_three", 3),
    bin<int>("cvp3_four", 4),
    bin<int>("cvp3_five", 5),
    bin<int>("cvp3_six", 6),
    bin<int>("cvp3_seven", 7),
    bin<int>("cvp3_eigth", 8),
    bin<int>("cvp3_nine", 9),
    bin<int>("cvp3_ten", 10),
    bin<int>("cvp3_eleven", 11),
    bin<int>("cvp3_twelve", 12),
    bin<int>("cvp3_thirteen",13),
    bin<int>("cvp3_fourteen", 14),
    bin<int>("cvp3_fifthteen", 15)
      
  };  
  
  cross<int,int,int> cross1 = cross<int,int, int> (this, "cross1", &cvp1, &cvp2, &cvp3);
};

TEST(cross_bins_filtering, binsof) {
    basic_cross_bins_filtering_test basic_cg_1;



    // TODO: update testcase to check the functionality of binsof, || and && operators


    // sample valid cross
    basic_cg_1.sample(1, 1, 11);

    // should be ~ .83
    EXPECT_GT(basic_cg_1.cross1.get_inst_coverage(), 0.82);
    EXPECT_LT(basic_cg_1.cross1.get_inst_coverage(),0.84);

    // sample that includes an ignore bin - cvp2's cvp2_five bin
    basic_cg_1.sample(1, 5, 11);
    EXPECT_GT(basic_cg_1.cross1.get_inst_coverage(), 0.82);
    EXPECT_LT(basic_cg_1.cross1.get_inst_coverage(),0.84);

    // sample that includes an out of range sample value for cvp 2
    basic_cg_1.sample(1, 1000, 11);
    EXPECT_GT(basic_cg_1.cross1.get_inst_coverage(), 0.82);
    EXPECT_LT(basic_cg_1.cross1.get_inst_coverage(),0.84);
    
    fc4sc::global::coverage_save("basic_cross_bin_filtering_1_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");


    basic_cg_1.sample(1, 2, 3);
    basic_cg_1.sample(2, 1, 2);
    basic_cg_1.sample(2, 2, 1);
    
    basic_cg_1.sample(1, 3, 3);
    //basic_cg_1.cross1.option.goal = 1;
    //EXPECT_EQ(basic_cg_1.cross1.get_inst_coverage(), 100);

    fc4sc::global::coverage_save("basic_cross_bin_filtering_2_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");


}
