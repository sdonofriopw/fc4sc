#include "fc4sc.hpp"
#include "gtest/gtest.h"

class basic_cross_bins_filtering_test: public covergroup {
public:
  CG_CONS(basic_cross_bins_filtering_test){
    cvp1.option.weight = 0;
    cvp2.option.weight = 0;
  };

  int SAMPLE_POINT(sample_point_1, cvp1);
  int SAMPLE_POINT(sample_point_2, cvp2);
  int SAMPLE_POINT(sample_point_3, cvp3);

  void sample(const int& x, const int& y,const int& z) {
    this->sample_point_1 = x;
    this->sample_point_2 = y;
    this->sample_point_3 = z;
    covergroup::sample();
  }
  coverpoint<int> cvp1 = coverpoint<int> (this, "cvp1",
                                          bin<int>("cvp1_one", 1),
                                          bin<int>("cvp1_two", 2),
                                          bin<int>("cvp1_three", 3)
                                          );
  coverpoint<int> cvp2 = coverpoint<int> (this, "cvp2",
                                          bin<int>("cvp2_one", 1),
                                          bin<int>("cvp2_two", 2),
                                          bin<int>("cvp2_three", 3)
                                          );

  coverpoint<int> cvp3 = coverpoint<int> (this, "cvp3",
                                          bin<int>("cvp3_one", 1),
                                          bin<int>("cvp3_two", 2)
                                          );

  cross<int,int,int> cross1 = cross<int,int, int> (this, "cross1", &cvp1, &cvp2, &cvp3);
};

TEST(cross_bins_filtering, binsof) {
    basic_cross_bins_filtering_test basic_cg_1;


    
    // TODO: update testcase to check the functionality of binsof, || and && operators
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(0, 0, 0);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(1, 0, 0);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(0, 1, 0);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 0);

    basic_cg_1.sample(1, 2, 1);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 26.39);

    fc4sc::global::coverage_save("basic_cross_bin_filtering_sample_1_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");
    
    basic_cg_1.sample(1, 2, 2);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 11);

    fc4sc::global::coverage_save("basic_cross_bin_filtering_sample_2_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");
    
    basic_cg_1.cross1.option.goal = 11;
    EXPECT_EQ(basic_cg_1.cross1.get_inst_coverage(), 100);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 100);
    fc4sc::global::coverage_save("basic_cross_bin_filtering_sample_3_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");

    
    basic_cg_1.cross1.option.goal = 100;

    basic_cg_1.sample(2, 2, 2);
    EXPECT_EQ(basic_cg_1.cross1.get_inst_coverage(), 11.11);
    fc4sc::global::coverage_save("basic_cross_bin_filtering_sample_4_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");

    basic_cg_1.sample(2, 1, 1);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 25);

    basic_cg_1.sample(1, 1, 1);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 38);


    basic_cg_1.sample(1, 3, 3);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 50);

    basic_cg_1.sample(2, 3, 2);
    EXPECT_EQ(basic_cg_1.get_inst_coverage(), 50);
    
    fc4sc::global::coverage_save("basic_cross_bin_filtering_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".xml");


}
