#include "../src/CompressionPrediction.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iterator>

using namespace itp;
using namespace testing;

TEST(IncrementTest, main)
{
	std::vector<Symbol> vec(4);
	std::fill(begin(vec), end(vec), 0);
	std::vector<std::vector<Symbol>> expected_values = {
		{1, 0, 0, 0},
		{2, 0, 0, 0},
		{0, 1, 0, 0},
		{1, 1, 0, 0},
		{2, 1, 0, 0},
		{0, 2, 0, 0},
		{1, 2, 0, 0},
		{2, 2, 0, 0}};

	for (size_t i = 0; i < expected_values.size(); ++i)
	{
		Increment(vec, 0, 3);
		EXPECT_THAT(vec, ElementsAreArray(expected_values[i]));
	}

	for (size_t i = 0; i < 4; ++i)
	{
		Increment(vec, 0, 3);
	}
	EXPECT_THAT(vec, ElementsAre(0, 1, 1, 0));

	EXPECT_THROW(Increment(vec, 1, 3), std::invalid_argument);
	EXPECT_THROW(Increment(vec, 3, 1), std::invalid_argument);

	std::fill(begin(vec), end(vec), 0);
	for (size_t i = 0; i < 80; ++i)
	{
		Increment(vec, 0, 3);
	}
	EXPECT_THAT(vec, ElementsAre(2, 2, 2, 2));

	Increment(vec, 0, 3);
	EXPECT_THAT(vec, ElementsAre(0, 0, 0, 0));

	std::vector<Symbol> vec1(1);
	vec[0] = 0;
	for (size_t i = 0; i < 256; ++i)
	{
		EXPECT_EQ(vec1[0], i);
		Increment(vec1, 0, 256);
	}
}

TEST(WeightsGeneratorTest, GenerateOnlyOneWeight_generate_Works)
{
	auto generator = std::make_shared<WeightsGenerator>();
	auto result = generator->Generate(1);
	EXPECT_EQ(result.size(), 1);
	EXPECT_DOUBLE_EQ(result[0], 1.);
}

TEST(WeightsGeneratorTest, GenerateSeveralWeights_generate_Works)
{
	auto generator = std::make_shared<WeightsGenerator>();
	auto result = generator->Generate(2);
	EXPECT_DOUBLE_EQ(result[0], .5);
	EXPECT_DOUBLE_EQ(result[1], .5);

	result = generator->Generate(5);
	EXPECT_DOUBLE_EQ(result[0], .2);
	EXPECT_DOUBLE_EQ(result[1], .2);
	EXPECT_DOUBLE_EQ(result[2], .2);
	EXPECT_DOUBLE_EQ(result[3], .2);
	EXPECT_DOUBLE_EQ(result[4], .2);
}

TEST(DifferentizerTest, RealTimeSeriesZeroDifference_diff_Works)
{
	PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
	auto df = DiffN(PreprocessedTimeSeries<Double, Double>(v), 0);
	EXPECT_EQ(df.size(), v.size());
	for (size_t i = 0; i < v.size(); ++i)
	{
		EXPECT_NEAR(v[i], df[i], 1e-5);
	}
}

TEST(DifferentizerTest, RealTimeSeriesTime_SeriesDifference_diff_Works)
{
	PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
	PlainTimeSeries<Double> v_expected = {1.2, 1.1, -4.8, 3.2, -2.1, 2.3, 4.3, -2.8};
	auto df = DiffN(PreprocessedTimeSeries<Double, Double>(v), 1);
	EXPECT_EQ(df.size(), v_expected.size());
	for (size_t i = 0; i < v_expected.size(); ++i)
	{
		EXPECT_NEAR(v_expected[i], df[i], 1e-5);
	}
}

TEST(DifferentizerTest, RealTimeSeriesIntegration_InfoDifference_diff_Works)
{
	PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
	PlainTimeSeries<Double> vv_expected = {-0.1, -5.9, 8.0, -5.3, 4.4, 2.0, -7.1};
	auto df = DiffN(PreprocessedTimeSeries<Double, Double>(v), 2);
	EXPECT_EQ(df.size(), vv_expected.size());
	for (size_t i = 0; i < vv_expected.size(); ++i)
	{
		EXPECT_NEAR(vv_expected[i], df[i], 1e-5);
	}
}

TEST(DifferentizerTest, RealTimeSeriesZeroDifferentiated_integrate_Works)
{
	PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
	auto df = DiffN(PreprocessedTimeSeries<Double, Double>(v), 0);
	std::string compressor1 = "zlib";
	std::string compressor2 = "bzip2";
	Forecast<Double> fake_forecast;
	fake_forecast(compressor1, 0).point = 1.0;
	fake_forecast(compressor1, 1).point = 2.0;
	fake_forecast(compressor2, 0).point = 3.0;
	fake_forecast(compressor2, 1).point = 4.0;
	Integrate(fake_forecast);
	ASSERT_EQ(fake_forecast.FactorsSize(), 2);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 0).point, 1.0);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 1).point, 2.0);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 0).point, 3.0);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 1).point, 4.0);
}

TEST(DifferentizerTest, RealTimeSeriesTime_SeriesDifferentiated_integrate_Works)
{
	PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
	auto df = DiffN(PreprocessedTimeSeries<Double, Double>(v), 1);
	std::string compressor1 = "zlib";
	std::string compressor2 = "bzip2";
	Forecast<Double> fake_forecast;
	fake_forecast(compressor1, 0).point = 1.0;
	fake_forecast(compressor1, 1).point = 2.0;
	fake_forecast(compressor2, 0).point = 3.0;
	fake_forecast(compressor2, 1).point = 4.0;
	fake_forecast.PushLastDiffValue(4.9);

	Integrate(fake_forecast);
	ASSERT_EQ(fake_forecast.FactorsSize(), 2);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 0).point, 5.9);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 1).point, 7.9);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 0).point, 7.9);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 1).point, 11.9);
}

TEST(DifferentizerTest, RealTimeSeriesIntegration_InfoDifferentiated_integrate_Works)
{
	PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
	auto df = DiffN(PreprocessedTimeSeries<Double, Double>(v), 2);
	std::string compressor1 = "zlib";
	std::string compressor2 = "bzip2";
	Forecast<Double> fake_forecast;
	fake_forecast(compressor1, 0).point = 1.0;
	fake_forecast(compressor1, 1).point = 2.0;
	fake_forecast(compressor2, 0).point = 3.0;
	fake_forecast(compressor2, 1).point = 4.0;
	fake_forecast.PushLastDiffValue(4.9);
	fake_forecast.PushLastDiffValue(-2.8);

	Integrate(fake_forecast);
	ASSERT_EQ(fake_forecast.FactorsSize(), 2);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 0).point, 3.1);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 1).point, 3.3);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 0).point, 5.1);
	EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 1).point, 9.3);
}

TEST(DataFrameTest, Constructors)
{
	std::vector<std::string> compressors{"gzip", "bzip2", "rp"};
	DataFrame<std::string, int, double> df(begin(compressors), end(compressors));
	EXPECT_EQ(df.GetIndex(), compressors);
	EXPECT_TRUE(df.GetFactors().empty());
	EXPECT_EQ(df.FactorsSize(), 0);
	EXPECT_EQ(df.IndexSize(), 3);

	std::vector<int> steps{1, 2, 3, 4, 5, 6};
	DataFrame<std::string, int, double> df1(begin(compressors), end(compressors), begin(steps), end(steps));
	EXPECT_EQ(df1.GetIndex(), compressors);
	EXPECT_EQ(df1.GetFactors(), steps);
	EXPECT_EQ(df1.IndexSize(), 3);
	EXPECT_EQ(df1.FactorsSize(), 6);

	DataFrame<Continuation<Symbol>, int, double> df2(Continuations_generator<Symbol>(4, 2), 16.);
	EXPECT_EQ(df2.IndexSize(), 16);

	DataFrame<int, int, int> df4{{1, 2, 3, 10}, {3, 4, 2, 1}};
	EXPECT_EQ(df4.GetIndex(), std::vector<int>({1, 2, 3, 10}));
	EXPECT_EQ(df4.GetFactors(), std::vector<int>({3, 4, 2, 1}));
}

TEST(DataFrameTest, Indexing)
{
	DataFrame<std::string, int, double> df({"gzip", "bzip2"}, {3, 1, 2});
	ASSERT_EQ(df.IndexSize(), 2);
	ASSERT_EQ(df.FactorsSize(), 3);

	df("brotli", 3) = 0.1;
	EXPECT_EQ(df.IndexSize(), 3);
	EXPECT_EQ(df.FactorsSize(), 3);
	EXPECT_EQ(df.GetIndex(), std::vector<std::string>({"gzip", "bzip2", "brotli"}));
	EXPECT_EQ(df.GetFactors(), std::vector<int>({3, 1, 2}));
}

TEST(DataFrameTest, Join)
{
	DataFrame<std::string, int, double> df;

	df.AddFactor(1);
	df.AddFactor(2);
	df.AddIndex("gzip");
	df("gzip", 1) = 0.5;
	df("gzip", 2) = 0.4;

	DataFrame<std::string, int, double> df1;
	df1.AddFactor(3);
	df1.AddIndex("gzip");
	df1("gzip", 3) = 0.9;
	df.Join(df1);

	EXPECT_EQ(df.IndexSize(), 1);
	EXPECT_EQ(df.FactorsSize(), 3);
	EXPECT_EQ(df.GetIndex(), std::vector<std::string>({"gzip"}));
	EXPECT_EQ(df.GetFactors(), std::vector<int>({1, 2, 3}));
}

TEST(DataFrameIteratorTest, main)
{
	DataFrame<std::string, int, int> df;
	df("gzip", 0) = 0;
	df("gzip", 1) = 1;
	df("bip2", 0) = 2;
	df("bip2", 1) = 3;
	size_t i = 0;
	for (auto iter = df.begin(); iter != df.end(); ++iter, ++i)
	{
		EXPECT_EQ(*iter, i);
	}

	i = 0;
	for (const auto& value : df)
	{
		EXPECT_EQ(value, i++);
	}
}

TEST(ContinuationTest, main)
{
	Continuation<Symbol> c(2, 4);
	for (size_t i = 0; i < c.size(); ++i)
	{
		EXPECT_EQ(c[i], 0);
	}
	EXPECT_EQ(c.size(), 4);
	EXPECT_EQ(c.get_alphabet_size(), 2);
	EXPECT_EQ(c.is_init(), true);
	EXPECT_EQ(c.overflow(), false);

	++c;
	EXPECT_EQ(c[0], 1);
	EXPECT_EQ(c[1], 0);
	EXPECT_EQ(c[2], 0);
	EXPECT_EQ(c[3], 0);

	c++;
	EXPECT_EQ(c[0], 0);
	EXPECT_EQ(c[1], 1);
	EXPECT_EQ(c[2], 0);
	EXPECT_EQ(c[3], 0);

	Continuation<Symbol> c1(2);
	EXPECT_THROW(c1 < c, std::invalid_argument);
	EXPECT_THROW(c1 > c, std::invalid_argument);
	EXPECT_NE(c1, c);
	EXPECT_NE(c1, c);
	EXPECT_EQ(c1, c1);
	EXPECT_EQ(c, c);

	for (size_t i = 2; i < pow(c.get_alphabet_size(), c.size()); ++i)
	{
		EXPECT_FALSE(c.overflow());
		++c;
	}
	EXPECT_TRUE(c.overflow());

	Continuation<Symbol> c2 = {1, 2, 3, 4};
	EXPECT_EQ(c2.size(), 4);
	EXPECT_EQ(c2.get_alphabet_size(), 5);
	for (size_t i = 0; i < 4; ++i)
	{
		EXPECT_EQ(c2[i], i + 1);
	}

	Continuation<Symbol> c3(256, 1);
	for (size_t i = 0; i < 256; ++i)
	{
		EXPECT_EQ(c3++, Continuation<Symbol>({static_cast<unsigned char>(i)}));
	}
}

TEST(CodesLengthsComputerTest, ComputeLengthsForAllContinuations_ComputeContinuationsDistribution_ComputedCorrectly)
{
	auto history = PreprocessedTimeSeries<Symbol, Symbol>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1};
	history.SetSamplingAlphabet(2);

	size_t length_of_continuation{3};
	const CompressorNames compressor_names = {"zstd", "ppmd"};

	CodeLengthsComputer<Symbol> computer{MakeStandardCompressorsPool()};
	const auto result = computer.ComputeContinuationsDistribution(history, length_of_continuation, compressor_names);

	ASSERT_EQ(8, result.IndexSize());

	Continuation<Symbol> c(history.GetSamplingAlphabet(), length_of_continuation);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 112.);

	++c;
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 112.);

	++c;
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 120.);

	++c;
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 112.);

	++c;
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 112.);

	++c;
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 112.);

	++c;
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 112.);

	++c;
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "zstd")), 176.);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c, "ppmd")), 104.);
}

class TablesConvertersTest : public ::testing::Test
{
protected:
	TablesConvertersTest()
	{
		Continuation<Symbol> c(2, 3);
		test_table(c, compressor1) = 2;
		test_table(c++, compressor2) = 3;
		test_table(c, compressor1) = 4;
		test_table(c++, compressor2) = 2;
		test_table(c, compressor1) = 2;
		test_table(c++, compressor2) = 1;
		test_table(c, compressor1) = 3;
		test_table(c++, compressor2) = 3;
		test_table(c, compressor1) = 4;
		test_table(c++, compressor2) = 6;
		test_table(c, compressor1) = 4;
		test_table(c++, compressor2) = 4;
		test_table(c, compressor1) = 3;
		test_table(c++, compressor2) = 3;
		test_table(c, compressor1) = 2;
		test_table(c++, compressor2) = 3;
	}

	std::string compressor1 = "zlib";
	std::string compressor2 = "ppmd";
	ContinuationsDistribution<Symbol> test_table;
};

TEST_F(TablesConvertersTest, TableWithCodeLengthsIsGiven_to_code_probabilities_ConvertedCorrectly)
{
	Continuation<Symbol> c1(2, 3);
	ToCodeProbabilities(begin(test_table), end(test_table));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -2));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -3));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -4));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -2));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -2));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -1));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -3));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -3));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -4));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -6));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -4));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -4));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -3));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -3));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1, compressor1)), pow(2, -2));
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c1++, compressor2)), pow(2, -3));
}

TEST_F(TablesConvertersTest, TableWithCodeProbabilitiesIsGiven_to_probabilities_ConvertedCorrectly)
{
	Continuation<Symbol> c1(2, 3);
	ToCodeProbabilities(begin(test_table), end(test_table));
	auto result = ToProbabilities(std::move(test_table));

	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -2) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -3) / 1.328125);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -4) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -2) / 1.328125);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -2) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -1) / 1.328125);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -3) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -3) / 1.328125);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -4) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -6) / 1.328125);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -4) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -4) / 1.328125);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -3) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -3) / 1.328125);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1, compressor1)), pow(2, -2) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c1++, compressor2)), pow(2, -3) / 1.328125);
}

TEST_F(TablesConvertersTest, TableWithProbabilitiesIsGiven_cumulated_for_step_SummedUpCorrectly)
{
	ToCodeProbabilities(begin(test_table), end(test_table));
	auto probabilities_table = ToProbabilities(test_table);
	auto t1 = CumulatedForStep(probabilities_table, 0);

	EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor1)), (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875);
	EXPECT_DOUBLE_EQ(
		static_cast<Double>(t1(1, compressor1)),
		(1 - (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875));

	EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor2)), (2 * pow(2, -3) + pow(2, -1) + pow(2, -6)) / 1.328125);
	EXPECT_DOUBLE_EQ(
		static_cast<Double>(t1(1, compressor2)),
		(1 - (2 * pow(2, -3) + pow(2, -1) + pow(2, -6)) / 1.328125));

	t1 = CumulatedForStep(probabilities_table, 1);
	EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor1)), (3 * pow(2, -4) + pow(2, -2)) / 1.1875);
	EXPECT_DOUBLE_EQ(static_cast<Double>(t1(1, compressor1)), (1 - (3 * pow(2, -4) + pow(2, -2)) / 1.1875));

	EXPECT_DOUBLE_EQ(
		static_cast<Double>(t1(0, compressor2)),
		(pow(2, -3) + pow(2, -2) + pow(2, -6) + pow(2, -4)) / 1.328125);
	EXPECT_DOUBLE_EQ(
		static_cast<Double>(t1(1, compressor2)),
		(1 - (pow(2, -3) + pow(2, -2) + pow(2, -6) + pow(2, -4)) / 1.328125));

	t1 = CumulatedForStep(probabilities_table, 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor1)), (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875);
	EXPECT_DOUBLE_EQ(
		static_cast<Double>(t1(1, compressor1)),
		(1 - (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875));

	EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor2)), (2 * pow(2, -3) + pow(2, -1) + pow(2, -2)) / 1.328125);
	EXPECT_DOUBLE_EQ(
		static_cast<Double>(t1(1, compressor2)),
		(1 - (2 * pow(2, -3) + pow(2, -1) + pow(2, -2)) / 1.328125));
}

TEST_F(TablesConvertersTest, CodeProbabilitiesForTwoCompressorsIsGiven_max_with_weights_ProbabilitiesCombinedCorrectly)
{
	ToCodeProbabilities(begin(test_table), end(test_table));
	auto generator = std::make_shared<WeightsGenerator>();
	FormGroupForecasts(test_table, {{compressor1, compressor2}}, generator);

	auto compressors = test_table.GetFactors();
	EXPECT_EQ(compressors.size(), 3);
	EXPECT_EQ(test_table.IndexSize(), 8);
	auto compressor = compressors[2];
	Continuation<Symbol> c(2, 3);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -2) + pow(2, -3)) / 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -4) + pow(2, -2)) / 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -2) + pow(2, -1)) / 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -3) + pow(2, -3)) / 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -4) + pow(2, -6)) / 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -4) + pow(2, -4)) / 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -3) + pow(2, -3)) / 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(test_table(c++, compressor)), (pow(2, -2) + pow(2, -3)) / 2);
}

TEST(MergeTest, main)
{
	std::vector<std::string> compressors = {"gzip", "bzip2"};

	ContinuationsDistribution<Symbol> table1(Continuations_generator<Symbol>(2, 2), 4);
	ContinuationsDistribution<Symbol> table2(Continuations_generator<Symbol>(4, 2), 16);
	ContinuationsDistribution<Symbol> table3(Continuations_generator<Symbol>(8, 2), 64);

	table1.AddFactor(begin(compressors), end(compressors));
	table2.AddFactor(begin(compressors), end(compressors));
	table3.AddFactor(begin(compressors), end(compressors));

	Continuation<Symbol> c(2, 2);
	table1(c++, "gzip") = 0.25;
	table1(c++, "gzip") = 0.5;
	table1(c++, "gzip") = 0.75;
	table1(c++, "gzip") = 1.;

	c = Continuation<Symbol>(2, 2);
	table1(c++, "bzip2") = 0.2;
	table1(c++, "bzip2") = 0.4;
	table1(c++, "bzip2") = 0.6;
	table1(c++, "bzip2") = 0.8;

	c = Continuation<Symbol>(4, 2);
	table2(c++, "gzip") = 0.1;
	table2(c++, "gzip") = 0.2;
	table2(c++, "gzip") = 0.3;
	table2(c++, "gzip") = 0.4;
	table2(c++, "gzip") = 0.5;
	table2(c++, "gzip") = 0.6;
	table2(c++, "gzip") = 0.7;
	table2(c++, "gzip") = 0.8;
	table2(c++, "gzip") = 0.9;
	table2(c++, "gzip") = 1.0;
	table2(c++, "gzip") = 1.1;
	table2(c++, "gzip") = 1.2;
	table2(c++, "gzip") = 1.3;
	table2(c++, "gzip") = 1.4;
	table2(c++, "gzip") = 1.5;
	table2(c++, "gzip") = 1.6;

	c = Continuation<Symbol>(4, 2);
	table2(c++, "bzip2") = 1.1;
	table2(c++, "bzip2") = 1.2;
	table2(c++, "bzip2") = 1.3;
	table2(c++, "bzip2") = 1.4;
	table2(c++, "bzip2") = 1.5;
	table2(c++, "bzip2") = 1.6;
	table2(c++, "bzip2") = 1.7;
	table2(c++, "bzip2") = 1.8;
	table2(c++, "bzip2") = 1.9;
	table2(c++, "bzip2") = 2.0;
	table2(c++, "bzip2") = 2.1;
	table2(c++, "bzip2") = 2.2;
	table2(c++, "bzip2") = 2.3;
	table2(c++, "bzip2") = 2.4;
	table2(c++, "bzip2") = 2.5;
	table2(c++, "bzip2") = 2.6;

	size_t i = 0;
	for (const auto& continuation : table3.GetIndex())
	{
		table3(continuation, "gzip") = i;
		table3(continuation, "bzip2") = (i + 1);
		++i;
	}

	auto wgen = std::make_shared<CountableWeightsGenerator>();
	auto weights = wgen->Generate(3);
	std::vector<size_t> alphabets{2, 4, 8};
	auto result = Merge(std::vector<ContinuationsDistribution<Symbol>>{table1, table2, table3}, alphabets, weights);

	c = Continuation<Symbol>(8, 2);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c++, "gzip")), (0.25 * 0.5 + 0.1 * 1 / 6. + 0));
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c++, "gzip")), (0.25 * 0.5 + 0.1 * 1 / 6. + 1. / 3.));
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c++, "gzip")), (0.25 * 0.5 + 0.2 * 1 / 6. + 2. / 3.));
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c++, "gzip")), (0.25 * 0.5 + 0.2 * 1 / 6. + 3. / 3.));
	ASSERT_EQ(c[0], 4);
	ASSERT_EQ(c[1], 0);
	EXPECT_DOUBLE_EQ(static_cast<Double>(result(c++, "gzip")), (0.5 * 0.5 + 0.3 * 1 / 6. + 4. / 3.));
	// todo
}

class CustomCompressionMehtodsTest : public ::testing::Test
{
protected:
	CustomCompressionMehtodsTest()
		: ts1{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		, ts2{1, 2, 3, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6}
		, ts3{1, 2, 3, 4, 5, 2, 1, 2, 3, 4, 2, 3, 129, 230, 2, 3, 1, 2, 3, 4, 2, 3, 4}
		, compressors_{MakeStandardCompressorsPool()}
	{
		compressors_->SetAlphabetDescription({0, 255});
	}

	unsigned char ts1[12];
	unsigned char ts2[15];
	unsigned char ts3[23];

	CompressorsFacadePtr compressors_;
};

TEST_F(CustomCompressionMehtodsTest, RePair)
{
	// The sequences were compressed manually by the original Re-Pair program.
	EXPECT_EQ(compressors_->Compress("rp", ts1, 12), BytesToBits(9));
	EXPECT_EQ(compressors_->Compress("rp", ts2, 15), BytesToBits(17));
	EXPECT_EQ(compressors_->Compress("rp", ts3, 23), BytesToBits(25));
}

TEST_F(CustomCompressionMehtodsTest, Ppmd)
{
	// The sequences were compressed manually by the original ppmd program.
	EXPECT_EQ(compressors_->Compress("ppmd", ts1, 12), BytesToBits(10));
	EXPECT_EQ(compressors_->Compress("ppmd", ts2, 15), BytesToBits(17));
	EXPECT_EQ(compressors_->Compress("ppmd", ts3, 23), BytesToBits(21));
}

TEST_F(CustomCompressionMehtodsTest, Lcacomp)
{
	// The sequences were compressed manually by the original ppmd program.
	EXPECT_EQ(compressors_->Compress("lcacomp", ts1, 12), BytesToBits(16));
	EXPECT_EQ(compressors_->Compress("lcacomp", ts2, 15), BytesToBits(28));
	EXPECT_EQ(compressors_->Compress("lcacomp", ts3, 23), BytesToBits(32));
}

TEST_F(CustomCompressionMehtodsTest, Zstd)
{
	// The sequences were compressed manually by the original ppmd program.
	EXPECT_EQ(compressors_->Compress("zstd", ts1, 12), BytesToBits(16));
	EXPECT_EQ(compressors_->Compress("zstd", ts2, 15), BytesToBits(24));
	EXPECT_EQ(compressors_->Compress("zstd", ts3, 23), BytesToBits(32));
}

TEST_F(CustomCompressionMehtodsTest, Bzip2)
{
	// The sequences were compressed manually by the original ppmd program.
	EXPECT_EQ(compressors_->Compress("bzip2", ts1, 12), BytesToBits(37));
	EXPECT_EQ(compressors_->Compress("bzip2", ts2, 15), BytesToBits(43));
	EXPECT_EQ(compressors_->Compress("bzip2", ts3, 23), BytesToBits(50));
}

TEST_F(CustomCompressionMehtodsTest, OneByOne)
{
	EXPECT_EQ(compressors_->Compress("bzip2", ts1, 12), BytesToBits(37));
	EXPECT_EQ(compressors_->Compress("zstd", ts1, 12), BytesToBits(16));
	EXPECT_EQ(compressors_->Compress("lcacomp", ts3, 23), BytesToBits(32));
}

TEST(RealPointwisePredictorTest, RealTsWithZeroDifferenceThreeStepsForecast_predict_PredictionIsCorrect)
{
	PlainTimeSeries<Double> ts{3.4, 0.1, 3.9, 4.8, 1.5, 1.8, 2.0, 4.9, 5.1, 2.1};
	auto computer = std::make_shared<CodeLengthsComputer<Double>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<Double>>();
	auto partition_cardinality = 4u;
	auto horizont = 2u;
	CompressorNamesVec compressor_groups{{"zlib"}, {"rp"}, {"zlib", "rp"}};
	auto dpredictor = std::make_shared<RealDistributionPredictor<Double>>(computer, sampler, partition_cardinality);
	dpredictor->SetDifferenceOrder(0);
	BasicPointwisePredictor<Double, Double> ppredictor{dpredictor};
	const auto forecast = ppredictor.Predict(InitPreprocessedTs(ts), horizont, compressor_groups);

	EXPECT_NEAR(forecast("zlib", 0).point, 1.8615389823, 1e-5);
	EXPECT_NEAR(forecast("zlib", 1).point, 1.8730772603, 1e-5);

	EXPECT_NEAR(forecast("rp", 0).point, 1.8896412464, 1e-5);
	EXPECT_NEAR(forecast("rp", 1).point, 1.8727279427, 1e-5);

	EXPECT_NEAR(forecast("zlib_rp", 0).point, 1.8896412464, 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 1).point, 1.8727279427, 1e-5);
}

TEST(DiscretePointwisePredictorTest, DiscreteTsWithZeroDifferenceTwoStepsForecast_predict_PredictionIsCorrect)
{
	std::vector<unsigned char> ts{2, 0, 2, 3, 1, 1, 1, 3, 3, 1};
	auto computer = std::make_shared<CodeLengthsComputer<Double>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<Symbol>>();
	size_t horizont = 2u;
	const CompressorNamesVec compressor_groups{{"zlib", "rp"}};
	auto dpredictor = std::make_shared<DiscreteDistributionPredictor<Double, Symbol>>(computer, sampler);
	BasicPointwisePredictor<Double, Symbol> ppredictor{dpredictor};
	const auto forecast = ppredictor.Predict(itp::InitPreprocessedTs(ts), horizont, compressor_groups);
	std::vector<double> expected_forecast{1.0264274976, 1.0151519618};
	EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
}

TEST(DiscretePointwisePredictorTest, DiscreteTsWithZeroDifferenceOneStepForecast_predict_PredictionIsCorrect)
{
	std::vector<unsigned char> ts{0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2};
	auto computer = std::make_shared<CodeLengthsComputer<Double>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<Symbol>>();
	size_t horizont = 1u;
	const CompressorNamesVec compressor_groups{{"ppmd"}};
	auto dpredictor = std::make_shared<DiscreteDistributionPredictor<Double, Symbol>>(computer, sampler);
	BasicPointwisePredictor<Double, Symbol> ppredictor{dpredictor};
	const auto forecast = ppredictor.Predict(InitPreprocessedTs(ts), horizont, compressor_groups);
	std::vector<double> expected_forecast{1.9922028179};
	EXPECT_NEAR(forecast("ppmd", 0).point, expected_forecast[0], 1e-5);
}

TEST(MultialphabetSparsePredictorTest, RealTsWithZeroDifferenceAndTwoPartitions_predict_PredictionIsCorrect)
{
	std::vector<Double> ts{3.4, 0.1, 3.9, 4.8, 1.5, 1.8, 2.0, 4.9, 5.1, 2.1};
	auto computer = std::make_shared<CodeLengthsComputer<Double>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<Double>>();
	auto max_partition_cardinality = 4u;
	auto horizont = 2u;
	const CompressorNamesVec compressor_groups{{"zlib", "rp"}};
	auto dpredictor = std::make_shared<MultialphabetDistributionPredictor<Double>>(
		computer,
		sampler,
		max_partition_cardinality);
	dpredictor->SetDifferenceOrder(0);
	BasicPointwisePredictor<Double, Double> ppredictor{dpredictor};
	const auto forecast = ppredictor.Predict(InitPreprocessedTs(ts), horizont, compressor_groups);
	std::vector<double> expected_forecast{3.0934987622, 3.0934080567};
	EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
}

TEST(SparseMultialphabetPredictorTest, RealTimeSeriesWithZeroDifference_predict_PredictionIsCorrect)
{
	std::vector<Double> ts{3.4, 2.5, 0.1, 0.5, 3.9, 4.0, 4.8, 2.8, 1.5, 1.3, 1.8, 2.1, 2, 3.5, 4.9, 5.0, 5.1, 4.5, 2.1};
	auto computer = std::make_shared<CodeLengthsComputer<Double>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<Double>>();
	size_t horizont{4};
	const CompressorNamesVec compressor_groups{{"zlib", "rp"}};
	size_t max_quants_count = 4;
	auto dpredictor = std::make_shared<MultialphabetDistributionPredictor<Double>>(computer, sampler, max_quants_count);
	size_t sparse = 2;
	auto ppredictor = std::make_shared<BasicPointwisePredictor<Double, Double>>(dpredictor);
	SparsePredictor<Double, Double> sparse_predictor{ppredictor, sparse};
	const auto forecast = sparse_predictor.Predict(InitPreprocessedTs(ts), horizont, compressor_groups);
	std::vector<double> expected_forecast{3.7364683941, 3.8542121847, 3.0934080567, 2.75};
	EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 2).point, expected_forecast[2], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 3).point, expected_forecast[3], 1e-5);
}

TEST(SparseMultialphabetPredictorTest, SparseM3CYear_predict_PredictionIsCorrect)
{
	PlainTimeSeries<Double> ts{
		940.66,
		1084.86,
		1244.98,
		1445.02,
		1683.17,
		2038.15,
		2342.52,
		2602.45,
		2927.87,
		3103.96,
		3360.27,
		3807.63,
		4387.88,
		4936.99};

	auto computer = std::make_shared<CodeLengthsComputer<Double>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<Double>>();
	size_t horizont = 6u;
	const CompressorNamesVec compressor_groups{{"zlib", "rp"}};
	size_t max_quants_count = 4;
	auto dpredictor = std::make_shared<MultialphabetDistributionPredictor<Double>>(computer, sampler, max_quants_count);
	dpredictor->SetDifferenceOrder(1);
	auto ppredictor = std::make_shared<BasicPointwisePredictor<Double, Double>>(dpredictor);
	size_t sparse = 2;
	SparsePredictor<Double, Double> sparse_predictor{ppredictor, sparse};
	const auto forecast = sparse_predictor.Predict(InitPreprocessedTs(ts), horizont, compressor_groups);

	PlainTimeSeries<Double> expected_forecast{
		5427.0124308808,
		5917.0363290153,
		6407.0594768841,
		6262.0988838384,
		6165.6275143097,
		6999.809906989};
	EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 2).point, expected_forecast[2], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 3).point, expected_forecast[3], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 4).point, expected_forecast[4], 1e-5);
	EXPECT_NEAR(forecast("zlib_rp", 5).point, expected_forecast[5], 1e-5);
}

TEST(RealMultialphabetVectorisedPredictorTest, WorksOnCorrectData)
{
	PlainTimeSeries<itp::VectorDouble> ts{
		{17374, 11910},
		{19421, 15659},
		{20582, 18295},
		{21182, 16411},
		{20227, 12566},
		{20779, 12079},
		{22390, 13845},
		{25608, 14750},
		{25197, 15769},
		{25302, 15186},
		{25043, 13256},
		{26899, 14352},
		{26803, 14429},
		{25600, 15473},
		{24569, 15871},
		{23005, 13237},
		{20863, 13034},
		{21298, 13085}};

	auto computer = std::make_shared<CodeLengthsComputer<VectorDouble>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<VectorDouble>>();
	size_t horizont = 2;
	const CompressorNamesVec compressor_groups{{"zlib", "rp"}};
	size_t max_quants_count = 4;
	auto dpredictor = std::make_shared<MultialphabetDistributionPredictor<VectorDouble>>(
		computer,
		sampler,
		max_quants_count);

	dpredictor->SetDifferenceOrder(1);
	auto ppredictor = std::make_shared<BasicPointwisePredictor<VectorDouble, VectorDouble>>(dpredictor);
	size_t sparse = 2;
	SparsePredictor<VectorDouble, VectorDouble> sparse_predictor{ppredictor, sparse};
	const auto forecast = sparse_predictor.Predict(InitPreprocessedTs(ts), horizont, compressor_groups);

	EXPECT_EQ(forecast("zlib_rp", 0).point.size(), 2);
	EXPECT_EQ(forecast("zlib_rp", 1).point.size(), 2);
}

TEST(RealMultialphabetVectorisedPredictorTest, ThrowsIfMaximalIntervalsCountExceeds256)
{
	PlainTimeSeries<itp::VectorDouble> ts{
		{17374, 11910},
		{19421, 15659},
		{20582, 18295},
		{21182, 16411},
		{20227, 12566},
		{20779, 12079},
		{22390, 13845},
		{25608, 14750},
		{25197, 15769},
		{25302, 15186},
		{25043, 13256},
		{26899, 14352},
		{26803, 14429},
		{25600, 15473},
		{24569, 15871},
		{23005, 13237},
		{20863, 13034},
		{21298, 13085}};

	auto computer = std::make_shared<CodeLengthsComputer<VectorDouble>>(MakeStandardCompressorsPool());
	auto sampler = std::make_shared<Sampler<VectorDouble>>();
	size_t horizont = 2;
	const CompressorNamesVec compressor_groups{{"zlib", "rp"}};
	size_t max_quants_count = 64;
	auto dpredictor = std::make_shared<MultialphabetDistributionPredictor<VectorDouble>>(
		computer,
		sampler,
		max_quants_count);

	dpredictor->SetDifferenceOrder(1);
	auto ppredictor = std::make_shared<BasicPointwisePredictor<VectorDouble, VectorDouble>>(dpredictor);
	size_t sparse = 2;
	SparsePredictor<VectorDouble, VectorDouble> sparse_predictor{ppredictor, sparse};

	EXPECT_THROW(sparse_predictor.Predict(InitPreprocessedTs(ts), horizont, compressor_groups), IntervalsCountError);
}
