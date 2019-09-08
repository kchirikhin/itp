#include <builders.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gtest_extensions.h"

using namespace testing;

class KIndexDataTest : public Test {
protected:
  std::vector<unsigned char> ts {2,1,1,1,1,1,2,2,3,1,1,2,2,2,3,4,5,3,2,3,3,1,1,0,1,1,2,3,4,5,3,4,5,3,2,2,2,2,2,3,2,3,3,4,2,3,3,1,2,2,6,6,4,2,2,3,4,4,3,2,2,2,2,3,2,2,2,2,2,1,1,2,2,3,1,2,2,3,1,1,3,2,3,2,2,1,2,3,0,1,1,2,3,2,2,2,2,2,1,1,3,3,2,2,2,1,1,2,3,1,2,1,0,1,1,1,3,2,0,0,1,3,2,1,2,1,2,1,3,3,1,2,1,0,0,2,0,0,0,1,1,1,1,2,2,0,0,1,1,1,1,2,1,0,1,1,0,0,0,0,1,2,2,1,2,4,3,3,4,3,2,3,3,1,2,2,3,3,2,2,2,2,2,1,0,1,1,1,2,2,2,2,1,2,1,1,1,1,1,3,3,3,3,4,3,3,2,2,3,3,3,2,2,2,2,3,3,2,3,2,1,1,0,1,2,1,1,2,2,2,4,3,2,3,2,1,1,2,3,3,1,2,1,1,2,2,2,4,4,3,3,2,2,3,4,5,5,5,4,4,3,4,4,3,4,4,3,2,3,3,2,3,2,2,1,2,3,3,1,1,2,3,1,2,3,3,3,2,0,1,2,3,2,1,1,0,1,1,3,3,2,3,2,1,2,3,3,2,1,2,3,3,3,1,1,0,2,2,2,2,1,2,1,0,1,2,2,3,1,3,1,2,2,1,0,0,1,1,0,1,2,1,1,1,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0,1,0,2,2,1,3,2,2,1,0,0,1,1,2,1,2,3,4,3,3,4,3,3,4,5,5,4,3,3,2,2,3,3,3,3,2,2,2,2,2,2,1,2,2,2,2,1,1,2,3,3,1,1,2,1,1,2,2,2,1,2,1,1,1,2,3,3,3,3,3,3,3,4,5,5,4,4,3,2,1,3,1,1,3,2,3,3,2,0,2,0,1,1,1,0,0,1,0,0,0,0,1,1,0,1,1,3,2,2,2,3,4,3,5,5,5,4,2,1,2,3,4,4,5,5,4,4,3,2,3,3,2,2,2,1,2,3,2,1,2,3,3,0,2,3,1,0,0,1,2,2,2,1,1,3,4,2,2,2,2,3,4,4,3,3,3,4,3,4,3,2,3,3,4,2,2,0,3,2,0,0,1,1,0,1,1,1,2,1,1,0,1,0,0,3,3,2,2,3,2,1,2,3,2,2,2,2,1,0,1,0,0,2};
  int sparse = 8;
  size_t difference = 0;
  size_t horizont = 24;
};

/*TEST_F(KIndexDataTest, PureAutomata) {
  itp::Names groups = {"automation"};
  auto res = make_forecast_discrete(ts, groups, horizont, difference, sparse);
  ASSERT_EQ(res.size(), 1);
  EXPECT_EQ(res["automation"].size(), 24);
  }*/

TEST_F(KIndexDataTest, PureCompressor) {
  itp::Names groups = {"zlib"};
  auto res = make_forecast_discrete(ts, groups, horizont, difference, sparse);
  ASSERT_EQ(res.size(), 1);
  EXPECT_EQ(res["zlib"].size(), horizont);
  for (size_t i = 0; i < horizont; ++i) {
    EXPECT_FALSE(std::isnan(res["zlib"][i]));
  }
}

// Recalculate
/*TEST_F(KIndexDataTest, CompressorWithAutomation) {
  itp::Names groups = {"zlib_automation"};
  auto res = make_forecast_discrete(ts, groups, horizont, difference, sparse);
  EXPECT_EQ(res.size(), 1);
  EXPECT_EQ(res["zlib_automation"].size(), 24);
  for (size_t i = 0; i < horizont; ++i) {
    EXPECT_FALSE(std::isnan(res["zlib_automation"][i]));
  }
  }*/

class BasicDataTest : public Test {
protected:
  std::vector<unsigned char> ts {1,0,1,1,0,1,1,1,0,1,1,1,0};
  int sparse = -1;
  size_t difference = 0;
  size_t horizont = 2;
};

TEST_F(BasicDataTest, PureCompressor) {
  itp::Names groups = {"zlib"};
  auto res = make_forecast_discrete(ts, groups, horizont, difference, sparse);
  EXPECT_EQ(res.size(), groups.size());
  EXPECT_EQ(res["zlib"].size(), horizont);
}

class ForecastFromFileTest : public Test {
protected:
  std::string fname = "kindex_ts_sh.dat";
  int sparse = -1;
  size_t difference = 0;
  size_t horizont = 3;
};

/*TEST(MakeForecastDiscreteTest, KIndexData_automata_zlib) {
  std::vector<unsigned char> ts {2,1,1,1,1,1,2,2,3,1,1,2,2,2,3,4,5,3,2,3,3,1,1,0,1,1,2,3,4,5,3,4,5,3,2,2,2,2,2,3,2,3,3,4,2,3,3,1,2,2,6,6,4,2,2,3,4,4,3,2,2,2,2,3,2,2,2,2,2,1,1,2,2,3,1,2,2,3,1,1,3,2,3,2,2,1,2,3,0,1,1,2,3,2,2,2,2,2,1,1,3,3,2,2,2,1,1,2,3,1,2,1,0,1,1,1,3,2,0,0,1,3,2,1,2,1,2,1,3,3,1,2,1,0,0,2,0,0,0,1,1,1,1,2,2,0,0,1,1,1,1,2,1,0,1,1,0,0,0,0,1,2,2,1,2,4,3,3,4,3,2,3,3,1,2,2,3,3,2,2,2,2,2,1,0,1,1,1,2,2,2,2,1,2,1,1,1,1,1,3,3,3,3,4,3,3,2,2,3,3,3,2,2,2,2,3,3,2,3,2,1,1,0,1,2,1,1,2,2,2,4,3,2,3,2,1,1,2,3,3,1,2,1,1,2,2,2,4,4,3,3,2,2,3,4,5,5,5,4,4,3,4,4,3,4,4,3,2,3,3,2,3,2,2,1,2,3,3,1,1,2,3,1,2,3,3,3,2,0,1,2,3,2,1,1,0,1,1,3,3,2,3,2,1,2,3,3,2,1,2,3,3,3,1,1,0,2,2,2,2,1,2,1,0,1,2,2,3,1,3,1,2,2,1,0,0,1,1,0,1,2,1,1,1,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0,1,0,2,2,1,3,2,2,1,0,0,1,1,2,1,2,3,4,3,3,4,3,3,4,5,5,4,3,3,2,2,3,3,3,3,2,2,2,2,2,2,1,2,2,2,2,1,1,2,3,3,1,1,2,1,1,2,2,2,1,2,1,1,1,2,3,3,3,3,3,3,3,4,5,5,4,4,3,2,1,3,1,1,3,2,3,3,2,0,2,0,1,1,1,0,0,1,0,0,0,0,1,1,0,1,1,3,2,2,2,3,4,3,5,5,5,4,2,1,2,3,4,4,5,5,4,4,3,2,3,3,2,2,2,1,2,3,2,1,2,3,3,0,2,3,1,0,0,1,2,2,2,1,1,3,4,2,2,2,2,3,4,4,3,3,3,4,3,4,3,2,3,3,4,2,2,0,3,2,0,0,1,1,0,1,1,1,2,1,1,0,1,0,0,3,3,2,2,3,2,1,2,3,2,2,2,2,1,0,1,0,0,2};
  itp::Names groups = {"automation_zlib"};
  int sparse = 8;
  size_t difference = 0;
  size_t horizont = 24;
  auto res = make_forecast_discrete(ts, groups, horizont, difference, sparse);
  EXPECT_EQ(res.size(), 2);
  EXPECT_EQ(res["automation"].size(), 24);
  EXPECT_EQ(res["automation_zlib"].size(), 24);
  }*/

/*TEST(MakeForecastDiscreteTest, KIndexData_automata_zlib) {
  std::vector<unsigned char> ts {2,1,1,1,1,1,2,2,3,1,1,2,2,2,3,4,5,3,2,3,3,1,1,0,1,1,2,3,4,5,3,4,5,3,2,2,2,2,2,3,2,3,3,4,2,3,3,1,2,2,6,6,4,2,2,3,4,4,3,2,2,2,2,3,2,2,2,2,2,1,1,2,2,3,1,2,2,3,1,1,3,2,3,2,2,1,2,3,0,1,1,2,3,2,2,2,2,2,1,1,3,3,2,2,2,1,1,2,3,1,2,1,0,1,1,1,3,2,0,0,1,3,2,1,2,1,2,1,3,3,1,2,1,0,0,2,0,0,0,1,1,1,1,2,2,0,0,1,1,1,1,2,1,0,1,1,0,0,0,0,1,2,2,1,2,4,3,3,4,3,2,3,3,1,2,2,3,3,2,2,2,2,2,1,0,1,1,1,2,2,2,2,1,2,1,1,1,1,1,3,3,3,3,4,3,3,2,2,3,3,3,2,2,2,2,3,3,2,3,2,1,1,0,1,2,1,1,2,2,2,4,3,2,3,2,1,1,2,3,3,1,2,1,1,2,2,2,4,4,3,3,2,2,3,4,5,5,5,4,4,3,4,4,3,4,4,3,2,3,3,2,3,2,2,1,2,3,3,1,1,2,3,1,2,3,3,3,2,0,1,2,3,2,1,1,0,1,1,3,3,2,3,2,1,2,3,3,2,1,2,3,3,3,1,1,0,2,2,2,2,1,2,1,0,1,2,2,3,1,3,1,2,2,1,0,0,1,1,0,1,2,1,1,1,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0,1,0,2,2,1,3,2,2,1,0,0,1,1,2,1,2,3,4,3,3,4,3,3,4,5,5,4,3,3,2,2,3,3,3,3,2,2,2,2,2,2,1,2,2,2,2,1,1,2,3,3,1,1,2,1,1,2,2,2,1,2,1,1,1,2,3,3,3,3,3,3,3,4,5,5,4,4,3,2,1,3,1,1,3,2,3,3,2,0,2,0,1,1,1,0,0,1,0,0,0,0,1,1,0,1,1,3,2,2,2,3,4,3,5,5,5,4,2,1,2,3,4,4,5,5,4,4,3,2,3,3,2,2,2,1,2,3,2,1,2,3,3,0,2,3,1,0,0,1,2,2,2,1,1,3,4,2,2,2,2,3,4,4,3,3,3,4,3,4,3,2,3,3,4,2,2,0,3,2,0,0,1,1,0,1,1,1,2,1,1,0,1,0,0,3,3,2,2,3,2,1,2,3,2,2,2,2,1,0,1,0,0,2};
  itp::Names groups = {"zlib", "automata"};
  int sparse = 8;
  size_t difference = 0;
  size_t horizont = 24;
  auto res = make_forecast_discrete(ts, groups, horizont, difference, sparse);
  EXPECT_EQ(res.size(), 2);
  EXPECT_EQ(res["automata"].size(), 24);
  EXPECT_EQ(res["zlib"].size(), 24);
  }*/

TEST(ConvertorsOfMultivariateSeriesTest, ReturnsEmptySeriesOnEmptyInput) {
  EXPECT_THAT(Convert(std::vector<std::vector<double>>{}), IsEmpty());
  EXPECT_THAT(Convert(std::vector<itp::VectorDouble>{}), IsEmpty());
}

TEST(ConvertorsOfMultivariateSeriesTest, ConvertsFromInputFormatToInternalFormat) {
  std::vector<std::vector<itp::Double>> input_data { {1., 2., 3.}, {4., 5., 6.} };
  std::vector<itp::VectorDouble> output_data { {1., 4.}, {2., 5.}, {3., 6.} };
  auto res = Convert(input_data);

  ASSERT_EQ(output_data.size(), res.size());
  for (size_t i = 0; i < res.size(); ++i) {
    itp::ExpectDoubleContainersEq(output_data[i], res[i]);
  }
}

TEST(ConvertorsOfMultivariateSeriesTest, ConvertsFromInernalFormatToOutputFormat) {
  std::vector<itp::VectorDouble> internal_data { {1., 2.}, {3., 4.}, {5., 6.} };
  std::vector<std::vector<double>> expected_data { {1., 3., 5.}, {2., 4., 6.} };
  auto res = Convert(internal_data);

  ASSERT_EQ(expected_data.size(), res.size());
  for (size_t i = 0; i < res.size(); ++i) {
    itp::ExpectDoubleContainersEq(expected_data[i], res[i]);
  }
}
