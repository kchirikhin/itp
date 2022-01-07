#include "../src/PredictorPrivate.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "GtestExtensions.h"

using itp::ConcatenatedCompressorNamesVec;
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

	itp::InformationTheoreticPredictor predictor_;
};

TEST_F(KIndexDataTest, PureAutomaton)
{
	const ConcatenatedCompressorNamesVec compressor_groups_vec{"automation"};
	const auto res = predictor_.ForecastDiscrete(
		ts_,
		compressor_groups_vec,
		horizon_,
		difference_,
		sparse_);

	ASSERT_EQ(std::size(res), std::size(compressor_groups_vec));
	EXPECT_EQ(std::size(res.at("automation")), horizon_);
}

TEST_F(KIndexDataTest, PureCompressor)
{
	ConcatenatedCompressorNamesVec compressor_groups_vec{"zlib"};
	const auto res = predictor_.ForecastDiscrete(
		ts_,
		compressor_groups_vec,
		horizon_,
		difference_,
		sparse_);

	ASSERT_EQ(std::size(res), std::size(compressor_groups_vec));
	EXPECT_EQ(std::size(res.at("zlib")), horizon_);

	for (size_t i = 0; i < horizon_; ++i)
	{
		EXPECT_FALSE(std::isnan(res.at("zlib")[i]));
	}
}

TEST_F(KIndexDataTest, CompressorWithAutomation)
{
	ConcatenatedCompressorNamesVec groups = {"zlib_automation"};
	auto res = predictor_.ForecastDiscrete(ts_, groups, horizon_, difference_, sparse_);

	EXPECT_EQ(res.size(), 3);
	EXPECT_EQ(res["zlib_automation"].size(), 24);

	for (size_t i = 0; i < horizon_; ++i)
	{
		EXPECT_FALSE(std::isnan(res["zlib_automation"][i]));
	}
}

class BasicDataTest : public Test
{
protected:
	const ConcatenatedCompressorNamesVec compressor_groups_vec_{"zlib"};
	const int sparse_ = -1;
	const size_t difference_ = 0;
	const size_t horizon_ = 2;

	itp::InformationTheoreticPredictor predictor_;
};

TEST_F(BasicDataTest, PureCompressor)
{
	std::vector<unsigned char> ts{1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0};
	const auto res = predictor_.ForecastDiscrete(
		ts,
		compressor_groups_vec_,
		horizon_,
		difference_,
		sparse_);

	EXPECT_EQ(std::size(res), std::size(compressor_groups_vec_));
	EXPECT_EQ(std::size(res.at("zlib")), horizon_);
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
	const auto res = predictor_.ForecastMultialphabetVec(
		ts,
		compressor_groups_vec_,
		horizon_,
		difference_,
		max_quanta_count,
		sparse_);

	ASSERT_EQ(std::size(res), std::size(compressor_groups_vec_));
	ASSERT_EQ(std::size(res.at("zlib")), std::size(ts));
	EXPECT_EQ(std::size(res.at("zlib")[0]), horizon_);
	EXPECT_EQ(std::size(res.at("zlib")[1]), horizon_);
}

class MakeForecastDiscreteTest : public Test
{
protected:
	std::vector<unsigned char> ts_{2,1,1,1,1,1,2,2,3,1,1,2,2,2,3,4,5,3,2,3,3,1,1,0,1,1,2,3,4,5,3,4,5,3,2,2,2,2,2,3,2,3,3,4,2,3,3,1,2,2,6,6,4,2,2,3,4,4,3,2,2,2,2,3,2,2,2,2,2,1,1,2,2,3,1,2,2,3,1,1,3,2,3,2,2,1,2,3,0,1,1,2,3,2,2,2,2,2,1,1,3,3,2,2,2,1,1,2,3,1,2,1,0,1,1,1,3,2,0,0,1,3,2,1,2,1,2,1,3,3,1,2,1,0,0,2,0,0,0,1,1,1,1,2,2,0,0,1,1,1,1,2,1,0,1,1,0,0,0,0,1,2,2,1,2,4,3,3,4,3,2,3,3,1,2,2,3,3,2,2,2,2,2,1,0,1,1,1,2,2,2,2,1,2,1,1,1,1,1,3,3,3,3,4,3,3,2,2,3,3,3,2,2,2,2,3,3,2,3,2,1,1,0,1,2,1,1,2,2,2,4,3,2,3,2,1,1,2,3,3,1,2,1,1,2,2,2,4,4,3,3,2,2,3,4,5,5,5,4,4,3,4,4,3,4,4,3,2,3,3,2,3,2,2,1,2,3,3,1,1,2,3,1,2,3,3,3,2,0,1,2,3,2,1,1,0,1,1,3,3,2,3,2,1,2,3,3,2,1,2,3,3,3,1,1,0,2,2,2,2,1,2,1,0,1,2,2,3,1,3,1,2,2,1,0,0,1,1,0,1,2,1,1,1,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0,1,0,2,2,1,3,2,2,1,0,0,1,1,2,1,2,3,4,3,3,4,3,3,4,5,5,4,3,3,2,2,3,3,3,3,2,2,2,2,2,2,1,2,2,2,2,1,1,2,3,3,1,1,2,1,1,2,2,2,1,2,1,1,1,2,3,3,3,3,3,3,3,4,5,5,4,4,3,2,1,3,1,1,3,2,3,3,2,0,2,0,1,1,1,0,0,1,0,0,0,0,1,1,0,1,1,3,2,2,2,3,4,3,5,5,5,4,2,1,2,3,4,4,5,5,4,4,3,2,3,3,2,2,2,1,2,3,2,1,2,3,3,0,2,3,1,0,0,1,2,2,2,1,1,3,4,2,2,2,2,3,4,4,3,3,3,4,3,4,3,2,3,3,4,2,2,0,3,2,0,0,1,1,0,1,1,1,2,1,1,0,1,0,0,3,3,2,2,3,2,1,2,3,2,2,2,2,1,0,1,0,0,2};
	int sparse_ = 8;
	size_t difference_ = 0;
	size_t horizon_ = 24;

	itp::InformationTheoreticPredictor predictor_;
};

TEST_F(MakeForecastDiscreteTest, KIndexDataWithAutomataAndZlibTogether)
{
	const ConcatenatedCompressorNamesVec compressor_groups_vec = {"automation_zlib"};

	const auto res = predictor_.ForecastDiscrete(
		ts_,
		compressor_groups_vec,
		horizon_,
		difference_,
		sparse_);

	EXPECT_EQ(std::size(res), 3);
	EXPECT_EQ(std::size(res.at("automation")), 24);
	EXPECT_EQ(std::size(res.at("zlib")), 24);
	EXPECT_EQ(std::size(res.at("automation_zlib")), 24);
}

TEST_F(MakeForecastDiscreteTest, KIndexDataWithAutomataAndZlibSeparately)
{
	const ConcatenatedCompressorNamesVec compressor_groups_vec = {"zlib", "automation"};

	const auto res = predictor_.ForecastDiscrete(ts_, compressor_groups_vec, horizon_, difference_, sparse_);

	EXPECT_EQ(std::size(res), 2);
	EXPECT_EQ(std::size(res.at("automation")), 24);
	EXPECT_EQ(std::size(res.at("zlib")), 24);
}

TEST(ConvertorsOfMultivariateSeriesTest, ReturnsEmptySeriesOnEmptyInput)
{
	EXPECT_THAT(itp::Convert(std::vector<std::vector<double>>{}), IsEmpty());
	EXPECT_THAT(itp::Convert(std::vector<itp::VectorDouble>{}), IsEmpty());
}

TEST(ConvertorsOfMultivariateSeriesTest, ConvertsFromInputFormatToInternalFormat)
{
	const std::vector<std::vector<itp::Double>> input_data{{1., 2., 3.}, {4., 5., 6.}};
	const std::vector<itp::VectorDouble> output_data{{1., 4.}, {2., 5.}, {3., 6.}};
	const auto res = itp::Convert(input_data);

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
