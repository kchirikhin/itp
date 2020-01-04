#include <builders.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gtest_extensions.h"

using namespace testing;

class KIndexDataTest : public Test
{
protected:
	const std::vector<unsigned char> ts_{2, 1, 1, 1, 1, 1, 2, 2, 3, 1, 1, 2, 2, 2, 3, 4, 5, 3, 2, 3, 3, 1, 1, 0, 1, 1,
										 2, 3, 4, 5, 3, 4, 5, 3, 2, 2, 2, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 1, 2, 2, 6, 6,
										 4, 2, 2, 3, 4, 4, 3, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 1, 1, 2, 2, 3, 1, 2, 2, 3,
										 1, 1, 3, 2, 3, 2, 2, 1, 2, 3, 0, 1, 1, 2, 3, 2, 2, 2, 2, 2, 1, 1, 3, 3, 2, 2,
										 2, 1, 1, 2, 3, 1, 2, 1, 0, 1, 1, 1, 3, 2, 0, 0, 1, 3, 2, 1, 2, 1, 2, 1, 3, 3,
										 1, 2, 1, 0, 0, 2, 0, 0, 0, 1, 1, 1, 1, 2, 2, 0, 0, 1, 1, 1, 1, 2, 1, 0, 1, 1,
										 0, 0, 0, 0, 1, 2, 2, 1, 2, 4, 3, 3, 4, 3, 2, 3, 3, 1, 2, 2, 3, 3, 2, 2, 2, 2,
										 2, 1, 0, 1, 1, 1, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 3, 3, 3, 3, 4, 3, 3, 2, 2,
										 3, 3, 3, 2, 2, 2, 2, 3, 3, 2, 3, 2, 1, 1, 0, 1, 2, 1, 1, 2, 2, 2, 4, 3, 2, 3,
										 2, 1, 1, 2, 3, 3, 1, 2, 1, 1, 2, 2, 2, 4, 4, 3, 3, 2, 2, 3, 4, 5, 5, 5, 4, 4,
										 3, 4, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 2, 2, 1, 2, 3, 3, 1, 1, 2, 3, 1, 2, 3, 3,
										 3, 2, 0, 1, 2, 3, 2, 1, 1, 0, 1, 1, 3, 3, 2, 3, 2, 1, 2, 3, 3, 2, 1, 2, 3, 3,
										 3, 1, 1, 0, 2, 2, 2, 2, 1, 2, 1, 0, 1, 2, 2, 3, 1, 3, 1, 2, 2, 1, 0, 0, 1, 1,
										 0, 1, 2, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 2, 2, 1,
										 3, 2, 2, 1, 0, 0, 1, 1, 2, 1, 2, 3, 4, 3, 3, 4, 3, 3, 4, 5, 5, 4, 3, 3, 2, 2,
										 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 1, 2, 3, 3, 1, 1, 2, 1, 1, 2,
										 2, 2, 1, 2, 1, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 4, 5, 5, 4, 4, 3, 2, 1, 3, 1, 1,
										 3, 2, 3, 3, 2, 0, 2, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 3, 2, 2,
										 2, 3, 4, 3, 5, 5, 5, 4, 2, 1, 2, 3, 4, 4, 5, 5, 4, 4, 3, 2, 3, 3, 2, 2, 2, 1,
										 2, 3, 2, 1, 2, 3, 3, 0, 2, 3, 1, 0, 0, 1, 2, 2, 2, 1, 1, 3, 4, 2, 2, 2, 2, 3,
										 4, 4, 3, 3, 3, 4, 3, 4, 3, 2, 3, 3, 4, 2, 2, 0, 3, 2, 0, 0, 1, 1, 0, 1, 1, 1,
										 2, 1, 1, 0, 1, 0, 0, 3, 3, 2, 2, 3, 2, 1, 2, 3, 2, 2, 2, 2, 1, 0, 1, 0, 0, 2};

	const int sparse_ = 8;
	const size_t difference_ = 0;
	const size_t horizon_ = 24;
};

TEST_F(KIndexDataTest, PureAutomaton)
{
	const itp::Names groups{"automation"};
	const auto res = make_forecast_discrete(ts_, groups, horizon_, difference_, sparse_);

	ASSERT_EQ(res.size(), groups.size());
	EXPECT_EQ(res.at("automation").size(), horizon_);
}

TEST_F(KIndexDataTest, PureCompressor)
{
	itp::Names groups{"zlib"};
	const auto res = make_forecast_discrete(ts_, groups, horizon_, difference_, sparse_);

	ASSERT_EQ(res.size(), groups.size());
	EXPECT_EQ(res.at("zlib").size(), horizon_);

	for (size_t i = 0; i < horizon_; ++i)
	{
		EXPECT_FALSE(std::isnan(res.at("zlib")[i]));
	}
}

// Recalculate
/*TEST_F(KIndexDataTest, CompressorWithAutomation) {
  itp::Names groups = {"zlib_automation"};
  auto res = make_forecast_discrete(ts_, groups, horizon_, difference_, sparse_);
  EXPECT_EQ(res.size(), 1);
  EXPECT_EQ(res["zlib_automation"].size(), 24);
  for (size_t i = 0; i < horizon_; ++i) {
    EXPECT_FALSE(std::isnan(res["zlib_automation"][i]));
  }
  }*/

class BasicDataTest : public Test
{
protected:
	const itp::Names groups_{"zlib"};
	const int sparse_ = -1;
	const size_t difference_ = 0;
	const size_t horizon_ = 2;
};

TEST_F(BasicDataTest, PureCompressor)
{
	std::vector<unsigned char> ts{1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0};
	const auto res = make_forecast_discrete(ts, groups_, horizon_, difference_, sparse_);

	EXPECT_EQ(res.size(), groups_.size());
	EXPECT_EQ(res.at("zlib").size(), horizon_);
}

TEST_F(BasicDataTest, AllowsToForecastMultivariateSeries)
{
	const std::vector<std::vector<double>> ts
			{
					{17374, 19421, 20582, 21182, 20227, 20779, 22390, 25608, 25197, 25302, 25043, 26899, 26803, 25600, 24569,
							23005, 20863, 21298, 17648},
					{23138, 24791, 26990, 28389, 28993, 28269, 27906, 30136, 33056, 34249, 35073, 34955, 37588, 38295, 38387,
							39078, 38185, 34448, 32673}
			};
	const auto max_quanta_count = 8u;
	const auto res = make_forecast_multialphabet_vec(ts, groups_, horizon_, difference_, max_quanta_count, sparse_);

	ASSERT_EQ(res.size(), groups_.size());
	ASSERT_EQ(res.at("zlib").size(), ts.size());
	EXPECT_EQ(res.at("zlib")[0].size(), horizon_);
	EXPECT_EQ(res.at("zlib")[1].size(), horizon_);
}

/*class ForecastFromFileTest : public Test
{
protected:
	std::string fname = "kindex_ts_sh.dat";
	int sparse = -1;
	size_t difference = 0;
	size_t horizont = 3;
};*/

/*TEST(MakeForecastDiscreteTest, KIndexData_automata_zlib) {
  std::vector<unsigned char> ts_ {2,1,1,1,1,1,2,2,3,1,1,2,2,2,3,4,5,3,2,3,3,1,1,0,1,1,2,3,4,5,3,4,5,3,2,2,2,2,2,3,2,3,3,4,2,3,3,1,2,2,6,6,4,2,2,3,4,4,3,2,2,2,2,3,2,2,2,2,2,1,1,2,2,3,1,2,2,3,1,1,3,2,3,2,2,1,2,3,0,1,1,2,3,2,2,2,2,2,1,1,3,3,2,2,2,1,1,2,3,1,2,1,0,1,1,1,3,2,0,0,1,3,2,1,2,1,2,1,3,3,1,2,1,0,0,2,0,0,0,1,1,1,1,2,2,0,0,1,1,1,1,2,1,0,1,1,0,0,0,0,1,2,2,1,2,4,3,3,4,3,2,3,3,1,2,2,3,3,2,2,2,2,2,1,0,1,1,1,2,2,2,2,1,2,1,1,1,1,1,3,3,3,3,4,3,3,2,2,3,3,3,2,2,2,2,3,3,2,3,2,1,1,0,1,2,1,1,2,2,2,4,3,2,3,2,1,1,2,3,3,1,2,1,1,2,2,2,4,4,3,3,2,2,3,4,5,5,5,4,4,3,4,4,3,4,4,3,2,3,3,2,3,2,2,1,2,3,3,1,1,2,3,1,2,3,3,3,2,0,1,2,3,2,1,1,0,1,1,3,3,2,3,2,1,2,3,3,2,1,2,3,3,3,1,1,0,2,2,2,2,1,2,1,0,1,2,2,3,1,3,1,2,2,1,0,0,1,1,0,1,2,1,1,1,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0,1,0,2,2,1,3,2,2,1,0,0,1,1,2,1,2,3,4,3,3,4,3,3,4,5,5,4,3,3,2,2,3,3,3,3,2,2,2,2,2,2,1,2,2,2,2,1,1,2,3,3,1,1,2,1,1,2,2,2,1,2,1,1,1,2,3,3,3,3,3,3,3,4,5,5,4,4,3,2,1,3,1,1,3,2,3,3,2,0,2,0,1,1,1,0,0,1,0,0,0,0,1,1,0,1,1,3,2,2,2,3,4,3,5,5,5,4,2,1,2,3,4,4,5,5,4,4,3,2,3,3,2,2,2,1,2,3,2,1,2,3,3,0,2,3,1,0,0,1,2,2,2,1,1,3,4,2,2,2,2,3,4,4,3,3,3,4,3,4,3,2,3,3,4,2,2,0,3,2,0,0,1,1,0,1,1,1,2,1,1,0,1,0,0,3,3,2,2,3,2,1,2,3,2,2,2,2,1,0,1,0,0,2};
  itp::Names groups = {"automation_zlib"};
  int sparse_ = 8;
  size_t difference_ = 0;
  size_t horizon_ = 24;
  auto res = make_forecast_discrete(ts_, groups, horizon_, difference_, sparse_);
  EXPECT_EQ(res.size(), 2);
  EXPECT_EQ(res["automation"].size(), 24);
  EXPECT_EQ(res["automation_zlib"].size(), 24);
  }*/

/*TEST(MakeForecastDiscreteTest, KIndexData_automata_zlib) {
  std::vector<unsigned char> ts_ {2,1,1,1,1,1,2,2,3,1,1,2,2,2,3,4,5,3,2,3,3,1,1,0,1,1,2,3,4,5,3,4,5,3,2,2,2,2,2,3,2,3,3,4,2,3,3,1,2,2,6,6,4,2,2,3,4,4,3,2,2,2,2,3,2,2,2,2,2,1,1,2,2,3,1,2,2,3,1,1,3,2,3,2,2,1,2,3,0,1,1,2,3,2,2,2,2,2,1,1,3,3,2,2,2,1,1,2,3,1,2,1,0,1,1,1,3,2,0,0,1,3,2,1,2,1,2,1,3,3,1,2,1,0,0,2,0,0,0,1,1,1,1,2,2,0,0,1,1,1,1,2,1,0,1,1,0,0,0,0,1,2,2,1,2,4,3,3,4,3,2,3,3,1,2,2,3,3,2,2,2,2,2,1,0,1,1,1,2,2,2,2,1,2,1,1,1,1,1,3,3,3,3,4,3,3,2,2,3,3,3,2,2,2,2,3,3,2,3,2,1,1,0,1,2,1,1,2,2,2,4,3,2,3,2,1,1,2,3,3,1,2,1,1,2,2,2,4,4,3,3,2,2,3,4,5,5,5,4,4,3,4,4,3,4,4,3,2,3,3,2,3,2,2,1,2,3,3,1,1,2,3,1,2,3,3,3,2,0,1,2,3,2,1,1,0,1,1,3,3,2,3,2,1,2,3,3,2,1,2,3,3,3,1,1,0,2,2,2,2,1,2,1,0,1,2,2,3,1,3,1,2,2,1,0,0,1,1,0,1,2,1,1,1,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0,1,0,2,2,1,3,2,2,1,0,0,1,1,2,1,2,3,4,3,3,4,3,3,4,5,5,4,3,3,2,2,3,3,3,3,2,2,2,2,2,2,1,2,2,2,2,1,1,2,3,3,1,1,2,1,1,2,2,2,1,2,1,1,1,2,3,3,3,3,3,3,3,4,5,5,4,4,3,2,1,3,1,1,3,2,3,3,2,0,2,0,1,1,1,0,0,1,0,0,0,0,1,1,0,1,1,3,2,2,2,3,4,3,5,5,5,4,2,1,2,3,4,4,5,5,4,4,3,2,3,3,2,2,2,1,2,3,2,1,2,3,3,0,2,3,1,0,0,1,2,2,2,1,1,3,4,2,2,2,2,3,4,4,3,3,3,4,3,4,3,2,3,3,4,2,2,0,3,2,0,0,1,1,0,1,1,1,2,1,1,0,1,0,0,3,3,2,2,3,2,1,2,3,2,2,2,2,1,0,1,0,0,2};
  itp::Names groups = {"zlib", "automata"};
  int sparse_ = 8;
  size_t difference_ = 0;
  size_t horizon_ = 24;
  auto res = make_forecast_discrete(ts_, groups, horizon_, difference_, sparse_);
  EXPECT_EQ(res.size(), 2);
  EXPECT_EQ(res["automata"].size(), 24);
  EXPECT_EQ(res["zlib"].size(), 24);
  }*/

TEST(ConvertorsOfMultivariateSeriesTest, ReturnsEmptySeriesOnEmptyInput)
{
	EXPECT_THAT(Convert(std::vector<std::vector<double>>{}), IsEmpty());
	EXPECT_THAT(Convert(std::vector<itp::VectorDouble>{}), IsEmpty());
}

TEST(ConvertorsOfMultivariateSeriesTest, ConvertsFromInputFormatToInternalFormat)
{
	const std::vector<std::vector<itp::Double>> input_data{{1., 2., 3.}, {4., 5., 6.}};
	const std::vector<itp::VectorDouble> output_data{{1., 4.}, {2., 5.}, {3., 6.}};
	const auto res = Convert(input_data);

	ASSERT_EQ(output_data.size(), res.size());
	for (size_t i = 0; i < res.size(); ++i)
	{
		itp::ExpectDoubleContainersEq(output_data[i], res[i]);
	}
}

TEST(ConvertorsOfMultivariateSeriesTest, ConvertsFromInernalFormatToOutputFormat)
{
	const std::vector<itp::VectorDouble> internal_data{{1., 2.}, {3., 4.}, {5., 6.}};
	const std::vector<std::vector<double>> expected_data{{1., 3., 5.}, {2., 4., 6.}};
	const auto res = Convert(internal_data);

	ASSERT_EQ(expected_data.size(), res.size());
	for (size_t i = 0; i < res.size(); ++i)
	{
		itp::ExpectDoubleContainersEq(expected_data[i], res[i]);
	}
}
