#include <compressors.h>
#include <compression_prediction.h>
#include <continuation.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iterator>

using namespace itp;
using namespace testing;

TEST(IncrementTest, main) {
  std::vector<Symbol> vec(4);
  std::fill(begin(vec), end(vec), 0);
  std::vector<std::vector<Symbol>> expected_values = {
    {1, 0, 0, 0}, {2, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0},
    {2, 1, 0, 0}, {0, 2, 0, 0}, {1, 2, 0, 0}, {2, 2, 0, 0}
  };
  
  for (size_t i = 0; i < expected_values.size(); ++i) {
    increment(vec, 0, 3);
    EXPECT_THAT(vec, ElementsAreArray(expected_values[i]));
  }

  for (size_t i = 0; i < 4; ++i) {
    increment(vec, 0, 3);
  }
  EXPECT_THAT(vec, ElementsAre(0, 1, 1, 0));

  EXPECT_THROW(increment(vec, 1, 3), std::invalid_argument);
  EXPECT_THROW(increment(vec, 3, 1), std::invalid_argument);

  std::fill(begin(vec), end(vec), 0);
  for (size_t i = 0; i < 80; ++i) {
    increment(vec, 0, 3);
  }
  EXPECT_THAT(vec, ElementsAre(2, 2, 2, 2));

  increment(vec, 0, 3);
  EXPECT_THAT(vec, ElementsAre(0, 0, 0, 0));

  std::vector<Symbol> vec1(1);
  vec[0] = 0;
  for (size_t i = 0; i < 256; ++i) {
    EXPECT_EQ(vec1[0], i);
    increment(vec1, 0, 256);
  }
}

TEST(WeightsGeneratorTest, GenerateOnlyOneWeight_generate_Works) {
  Weights_generator_ptr generator = std::make_shared<Weights_generator>();
  auto result = generator->generate(1);
  EXPECT_EQ(result.size(), 1);
  EXPECT_DOUBLE_EQ(result[0], 1.);
}

TEST(WeightsGeneratorTest, GenerateSeveralWeights_generate_Works) {
  Weights_generator_ptr generator = std::make_shared<Weights_generator>();
  auto result = generator->generate(2);
  EXPECT_DOUBLE_EQ(result[0], .5);
  EXPECT_DOUBLE_EQ(result[1], .5);

  result = generator->generate(5);
  EXPECT_DOUBLE_EQ(result[0], .2);
  EXPECT_DOUBLE_EQ(result[1], .2);
  EXPECT_DOUBLE_EQ(result[2], .2);
  EXPECT_DOUBLE_EQ(result[3], .2);
  EXPECT_DOUBLE_EQ(result[4], .2);
}

TEST(DifferentizerTest, RealTimeSeriesZeroDifference_diff_Works) {
  PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
  auto df = diff_n(Preprocessed_tseries<Double, Double>(v), 0);
  EXPECT_EQ(df.size(), v.size());
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_NEAR(v[i], df[i], 1e-5);
  }
}

TEST(DifferentizerTest, RealTimeSeriesTime_SeriesDifference_diff_Works) {
  PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
  PlainTimeSeries<Double> v_expected = {1.2, 1.1, -4.8, 3.2, -2.1, 2.3, 4.3, -2.8};
  auto df = diff_n(Preprocessed_tseries<Double, Double>(v), 1);
  EXPECT_EQ(df.size(), v_expected.size());
  for (size_t i = 0; i < v_expected.size(); ++i) {
    EXPECT_NEAR(v_expected[i], df[i], 1e-5);
  }
}

TEST(DifferentizerTest, RealTimeSeriesIntegration_InfoDifference_diff_Works) {
  PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
  PlainTimeSeries<Double> vv_expected = {-0.1, -5.9, 8.0, -5.3, 4.4, 2.0, -7.1};
  auto df = diff_n(Preprocessed_tseries<Double, Double>(v), 2);
  EXPECT_EQ(df.size(), vv_expected.size());
  for (size_t i = 0; i < vv_expected.size(); ++i) {
    EXPECT_NEAR(vv_expected[i], df[i], 1e-5);
  }
}

TEST(DifferentizerTest, RealTimeSeriesZeroDifferentiated_integrate_Works) {
  PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
  auto df = diff_n(Preprocessed_tseries<Double, Double>(v), 0);
  std::string compressor1 = "zlib";
  std::string compressor2 = "bzip2";
  Forecast<Double> fake_forecast;
  fake_forecast(compressor1, 0).point = 1.0;
  fake_forecast(compressor1, 1).point = 2.0;
  fake_forecast(compressor2, 0).point = 3.0;
  fake_forecast(compressor2, 1).point = 4.0;
  integrate(fake_forecast);
  ASSERT_EQ(fake_forecast.factors_size(), 2);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 0).point, 1.0);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 1).point, 2.0);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 0).point, 3.0);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 1).point, 4.0);
}

TEST(DifferentizerTest, RealTimeSeriesTime_SeriesDifferentiated_integrate_Works) {
  PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
  auto df = diff_n(Preprocessed_tseries<Double, Double>(v), 1);
  std::string compressor1 = "zlib";
  std::string compressor2 = "bzip2";
  Forecast<Double> fake_forecast;
  fake_forecast(compressor1, 0).point = 1.0;
  fake_forecast(compressor1, 1).point = 2.0;
  fake_forecast(compressor2, 0).point = 3.0;
  fake_forecast(compressor2, 1).point = 4.0;
  fake_forecast.push_last_diff_value(4.9);

  integrate(fake_forecast);
  ASSERT_EQ(fake_forecast.factors_size(), 2);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 0).point, 5.9);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 1).point, 7.9);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 0).point, 7.9);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 1).point, 11.9);
}

TEST(DifferentizerTest, RealTimeSeriesIntegration_InfoDifferentiated_integrate_Works) {
  PlainTimeSeries<Double> v = {2.5, 3.7, 4.8, 0, 3.2, 1.1, 3.4, 7.7, 4.9};
  auto df = diff_n(Preprocessed_tseries<Double, Double>(v), 2);
  std::string compressor1 = "zlib";
  std::string compressor2 = "bzip2";
  Forecast<Double> fake_forecast;
  fake_forecast(compressor1, 0).point = 1.0;
  fake_forecast(compressor1, 1).point = 2.0;
  fake_forecast(compressor2, 0).point = 3.0;
  fake_forecast(compressor2, 1).point = 4.0;
  fake_forecast.push_last_diff_value(4.9);
  fake_forecast.push_last_diff_value(-2.8);

  integrate(fake_forecast);
  ASSERT_EQ(fake_forecast.factors_size(), 2);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 0).point, 3.1);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor1, 1).point, 3.3);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 0).point, 5.1);
  EXPECT_DOUBLE_EQ(fake_forecast(compressor2, 1).point, 9.3);
}

TEST(DataFrameTest, Constructors) {
  std::vector<std::string> compressors{"gzip", "bzip2", "rp"};
  Data_frame<std::string, int, double> df(begin(compressors), end(compressors));
  EXPECT_EQ(df.get_index(), compressors);
  EXPECT_TRUE(df.get_factors().empty());
  EXPECT_EQ(df.factors_size(), 0);
  EXPECT_EQ(df.index_size(), 3);

  std::vector<int> steps{1, 2, 3, 4, 5, 6};
  Data_frame<std::string, int, double> df1(begin(compressors), end(compressors), begin(steps), end(steps));
  EXPECT_EQ(df1.get_index(), compressors);
  EXPECT_EQ(df1.get_factors(), steps);
  EXPECT_EQ(df1.index_size(), 3);
  EXPECT_EQ(df1.factors_size(), 6);

  Data_frame<Continuation<Symbol>, int, double> df2(Continuations_generator<Symbol>(4, 2), 16.);
  EXPECT_EQ(df2.index_size(), 16);

  Data_frame<int, int, int> df4{{1, 2, 3, 10},
    {3, 4, 2, 1}};
  EXPECT_EQ(df4.get_index(), std::vector<int>({1, 2, 3, 10}));
  EXPECT_EQ(df4.get_factors(), std::vector<int>({3, 4, 2, 1}));
}

TEST(DataFrameTest, Indexing) {
  Data_frame<std::string, int, double> df({"gzip", "bzip2"}, {3, 1, 2});
  ASSERT_EQ(df.index_size(), 2);
  ASSERT_EQ(df.factors_size(), 3);

  df("brotli", 3) = 0.1;
  EXPECT_EQ(df.index_size(), 3);
  EXPECT_EQ(df.factors_size(), 3);
  EXPECT_EQ(df.get_index(), std::vector<std::string>({"gzip", "bzip2", "brotli"}));
  EXPECT_EQ(df.get_factors(), std::vector<int>({3, 1, 2}));
}

TEST(DataFrameTest, Join) {
  Data_frame<std::string, int, double> df;

  df.add_factor(1);
  df.add_factor(2);
  df.add_index("gzip");
  df("gzip", 1) = 0.5;
  df("gzip", 2) = 0.4;

  Data_frame<std::string, int, double> df1;
  df1.add_factor(3);
  df1.add_index("gzip");
  df1("gzip", 3) = 0.9;
  df.join(df1);

  EXPECT_EQ(df.index_size(), 1);
  EXPECT_EQ(df.factors_size(), 3);
  EXPECT_EQ(df.get_index(), std::vector<std::string>({"gzip"}));
  EXPECT_EQ(df.get_factors(), std::vector<int>({1, 2, 3}));
}

TEST(DataFrameIteratorTest, main) {
  Data_frame<std::string, int, int> df;
  df("gzip", 0) = 0;
  df("gzip", 1) = 1;
  df("bip2", 0) = 2;
  df("bip2", 1) = 3;
  size_t i = 0;
  for (auto iter = df.begin(); iter != df.end(); ++iter, ++i) {
    EXPECT_EQ(*iter, i);
  }

  i = 0;
  for (const auto &value : df) {
    EXPECT_EQ(value, i++);
  }
}

TEST(ContinuationTest, main) {
  Continuation<Symbol> c(2, 4);
  for (size_t i = 0; i < c.size(); ++i) {
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

  for (size_t i = 2; i < pow(c.get_alphabet_size(), c.size()); ++i) {
    EXPECT_FALSE(c.overflow());
    ++c;
  }
  EXPECT_TRUE(c.overflow());

  Continuation<Symbol> c2 = {1, 2, 3, 4};
  EXPECT_EQ(c2.size(), 4);
  EXPECT_EQ(c2.get_alphabet_size(), 5);
  for (size_t i = 0; i < 4; ++i) {
    EXPECT_EQ(c2[i], i + 1);
  }

  Continuation<Symbol> c3(256, 1);
  for (size_t i = 0; i < 256; ++i) {
    EXPECT_EQ(c3++, Continuation<Symbol>({static_cast<unsigned char>(i)}));
  }
}

TEST(CodesLengthsComputerTest,
     ComputeLengthsForAllContinuations_append_each_trajectory_and_compute_ComputedCorrectly) {
  PlainTimeSeries<Symbol> history {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1};
  size_t alphabet {2};
  size_t length_of_continuation {3};
  Names compressors_to_compute {"zstd", "ppmd"};

  Codes_lengths_computer<Symbol> computer;
  auto result = computer.append_each_trajectory_and_compute(history, alphabet,
                                                            length_of_continuation,
                                                            compressors_to_compute);

  ASSERT_EQ(8, result.index_size());

  Continuation<Symbol> c(alphabet, length_of_continuation);
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

TEST(CompressorsPoolTest, compression) {
  unsigned char ts[] {0, 1, 1, 0, 1, 3, 0, 0, 0};
  size_t expected_size = 18;
  size_t obtained_size = Compressors_pool::get_instance()("zstd", ts, sizeof(ts));
  EXPECT_EQ(obtained_size, expected_size);

  expected_size = 17;
  obtained_size = Compressors_pool::get_instance()("zlib", ts, sizeof(ts));
  EXPECT_EQ(obtained_size, expected_size);

  expected_size = 18;
  obtained_size = Compressors_pool::get_instance()("zstd", ts, sizeof(ts));
  EXPECT_EQ(obtained_size, expected_size);

  expected_size = 15;
  obtained_size = Compressors_pool::get_instance()("ppmd", ts, sizeof(ts));
  EXPECT_EQ(obtained_size, expected_size);
}

class TablesConvertersTest : public ::testing::Test {
 protected:
  TablesConvertersTest() {
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

TEST_F(TablesConvertersTest, TableWithCodeLengthsIsGiven_to_code_probabilities_ConvertedCorrectly) {
  Continuation<Symbol> c1(2, 3);
  to_code_probabilities(begin(test_table), end(test_table));
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

TEST_F(TablesConvertersTest, TableWithCodeProbabilitiesIsGiven_to_probabilities_ConvertedCorrectly) {
  Continuation<Symbol> c1(2, 3);
  to_code_probabilities(begin(test_table), end(test_table));
  auto result = to_probabilities(std::move(test_table));

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

TEST_F(TablesConvertersTest, TableWithProbabilitiesIsGiven_cumulated_for_step_SummedUpCorrectly) {
  to_code_probabilities(begin(test_table), end(test_table));
  auto probabilities_table = to_probabilities(test_table);
  auto t1 = cumulated_for_step(probabilities_table, 0);

  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor1)), (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(1, compressor1)), (1 - (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875));

  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor2)), (2 * pow(2, -3) + pow(2, -1) + pow(2, -6)) / 1.328125);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(1, compressor2)), (1 - (2 * pow(2, -3) + pow(2, -1) + pow(2, -6)) / 1.328125));

  t1 = cumulated_for_step(probabilities_table, 1);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor1)), (3 * pow(2, -4) + pow(2, -2)) / 1.1875);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(1, compressor1)), (1 - (3 * pow(2, -4) + pow(2, -2)) / 1.1875));

  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor2)), (pow(2, -3) + pow(2, -2) + pow(2, -6) + pow(2, -4)) / 1.328125);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(1, compressor2)), (1 - (pow(2, -3) + pow(2, -2) + pow(2, -6) + pow(2, -4)) / 1.328125));

  t1 = cumulated_for_step(probabilities_table, 2);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor1)), (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(1, compressor1)), (1 - (2 * pow(2, -2) + pow(2, -4) + pow(2, -3)) / 1.1875));

  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(0, compressor2)), (2 * pow(2, -3) + pow(2, -1) + pow(2, -2)) / 1.328125);
  EXPECT_DOUBLE_EQ(static_cast<Double>(t1(1, compressor2)), (1 - (2 * pow(2, -3) + pow(2, -1) + pow(2, -2)) / 1.328125));
}

TEST_F(TablesConvertersTest, CodeProbabilitiesForTwoCompressorsIsGiven_max_with_weights_ProbabilitiesCombinedCorrectly) {
  to_code_probabilities(begin(test_table), end(test_table));
  Weights_generator_ptr generator = std::make_shared<Weights_generator>();
  form_group_forecasts(test_table, std::vector<Names>{{compressor1, compressor2}}, generator);

  auto compressors = test_table.get_factors();
  EXPECT_EQ(compressors.size(), 3);
  EXPECT_EQ(test_table.index_size(), 8);
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

TEST(MergeTest, main) {
  std::vector<std::string> compressors = {"gzip", "bzip2"};

  ContinuationsDistribution<Symbol> table1(Continuations_generator<Symbol>(2, 2), 4);
  ContinuationsDistribution<Symbol> table2(Continuations_generator<Symbol>(4, 2), 16);
  ContinuationsDistribution<Symbol> table3(Continuations_generator<Symbol>(8, 2), 64);

  table1.add_factor(begin(compressors), end(compressors));
  table2.add_factor(begin(compressors), end(compressors));
  table3.add_factor(begin(compressors), end(compressors));

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
  for (const auto &continuation : table3.get_index()) {
    table3(continuation, "gzip") = i;
    table3(continuation, "bzip2") = (i + 1);
    ++i;
  }

  auto wgen = std::make_shared<Countable_weights_generator>();
  auto weights = wgen->generate(3);
  std::vector<size_t> alphabets{2, 4, 8};
  auto result = merge(std::vector<ContinuationsDistribution<Symbol>>{table1, table2, table3}, alphabets, weights);

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

/*TEST(ConcatenateUniqueNamesTest, main) {
  std::vector<Group> vec{{"gzip"},
  {"bzip2", "gzip"},
  {"rp",    "gzip", "lca"}};
  auto unique_names = concatenate_unique_names(vec);
  EXPECT_EQ(unique_names, Group({"bzip2", "gzip", "lca", "rp"}));
  }*/

/*TEST(ContinueOneStepTest, main) {
  std::vector<Continuation<Symbol>> conts{{1, 0, 0, 1},
  {0, 1, 1, 1}};
  auto result = continue_one_step(conts, 2);
  ASSERT_EQ(result.size(), 4);
  EXPECT_NE(std::find(begin(result), end(result), Continuation<Symbol>({1, 0, 0, 1, 0})), end(result));
  EXPECT_NE(std::find(begin(result), end(result), Continuation<Symbol>({1, 0, 0, 1, 1})), end(result));
  EXPECT_NE(std::find(begin(result), end(result), Continuation<Symbol>({0, 1, 1, 1, 0})), end(result));
  EXPECT_NE(std::find(begin(result), end(result), Continuation<Symbol>({0, 1, 1, 1, 1})), end(result));

  std::vector<Continuation<Symbol>> empty_vec;
  result = continue_one_step(empty_vec, 3);
  ASSERT_EQ(result.size(), 3);
  }

  TEST(FindTopContinuationsTest, main) {
  std::string compressor{"gzip"};
  ContinuationsDistribution t{Continuations_generator<Symbol>(3, 2), 9};
  t(Continuation<Symbol>({1, 2}), compressor) = 0.5;
  t(Continuation<Symbol>({2, 2}), compressor) = 0.1;
  t(Continuation<Symbol>({0, 0}), compressor) = 0.3;

  auto vec = find_top_continuations(t, 1., 2);
  EXPECT_EQ(vec.size(), 2);
  EXPECT_NE(std::find(begin(vec), end(vec), Continuation<Symbol>({1, 2})), end(vec));
  EXPECT_NE(std::find(begin(vec), end(vec), Continuation<Symbol>({0, 0})), end(vec));
  }*/

TEST(CompressionMethodsTest, main) {
  std::vector<Symbol> time_series1 = {0, 1, 2, 3, 0, 1, 2, 1, 3, 0, 3, 4, 5, 3, 2, 0, 1, 2, 3, 4};
  std::vector<Symbol> time_series2 = {0, 1, 2, 3, 0, 1, 2, 1, 3, 0, 3, 4, 5, 3, 2, 0, 1, 2, 3, 4, 10, 10, 129, 200,
                                      198, 232, 190, 42, 12, 23, 43, 54, 54, 32};

  /*size_t c1 = lcacomp_compress(time_series1.data(), time_series1.size());
    size_t c2 = lcacomp_compress(time_series1.data(), time_series1.size());
    size_t c3 = lcacomp_compress(time_series1.data(), time_series1.size());
    EXPECT_EQ(c1, c2);
    EXPECT_EQ(c2, c3);

    size_t c4 = lcacomp_compress(time_series2.data(), time_series2.size());
    size_t c5 = lcacomp_compress(time_series2.data(), time_series2.size());
    size_t c6 = lcacomp_compress(time_series2.data(), time_series2.size());
    EXPECT_EQ(c4, c5);
    EXPECT_EQ(c5, c6);
    BOOST_CHECK(c1 < c4);*/

  auto c1 = Compressors_pool::get_instance()("rp", time_series1.data(), time_series1.size());
  auto c2 = Compressors_pool::get_instance()("rp", time_series1.data(), time_series1.size());
  auto c3 = Compressors_pool::get_instance()("rp", time_series1.data(), time_series1.size());
  EXPECT_EQ(c1, c2);
  EXPECT_EQ(c2, c3);

  auto c4 = Compressors_pool::get_instance()("rp", time_series2.data(), time_series2.size());
  auto c5 = Compressors_pool::get_instance()("rp", time_series2.data(), time_series2.size());
  auto c6 = Compressors_pool::get_instance()("rp", time_series2.data(), time_series2.size());
  EXPECT_EQ(c4, c5);
  EXPECT_EQ(c5, c6);
  EXPECT_LE(c1, c4);

  /*c1 = sequitur::sequitur_compress(time_series1.data(), time_series1.size());
    c2 = sequitur::sequitur_compress(time_series1.data(), time_series1.size());
    c3 = sequitur::sequitur_compress(time_series1.data(), time_series1.size());
    EXPECT_EQ(c1, c2);
    EXPECT_EQ(c2, c3);

    c4 = sequitur::sequitur_compress(time_series2.data(), time_series2.size());
    c5 = sequitur::sequitur_compress(time_series2.data(), time_series2.size());
    c6 = sequitur::sequitur_compress(time_series2.data(), time_series2.size());
    EXPECT_EQ(c4, c5);
    EXPECT_EQ(c5, c6);*/
  //BOOST_CHECK(c1 < c4); // bug
}

class CustomCompressionMehtodsTest : public ::testing::Test {
 protected:
  CustomCompressionMehtodsTest()
      : ts1{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, ts2{1, 2, 3, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6},
        ts3{1, 2, 3, 4, 5, 2, 1, 2, 3, 4, 2, 3, 129, 230, 2, 3, 1, 2, 3, 4, 2, 3, 4} {
          // DO NOTHING
        }

  unsigned char ts1[12];
  unsigned char ts2[15];
  unsigned char ts3[23];
};

TEST_F(CustomCompressionMehtodsTest, RePair) {
  // The sequences were compressed manually by the original Re-Pair program.
  EXPECT_EQ(Compressors_pool::get_instance()("rp", ts1, 12), 9);
  EXPECT_EQ(Compressors_pool::get_instance()("rp", ts2, 15), 17);
  EXPECT_EQ(Compressors_pool::get_instance()("rp", ts3, 23), 25);
}

TEST_F(CustomCompressionMehtodsTest, Ppmd) {
  // The sequences were compressed manually by the original ppmd program.
  EXPECT_EQ(Compressors_pool::get_instance()("ppmd", ts1, 12), 10);
  EXPECT_EQ(Compressors_pool::get_instance()("ppmd", ts2, 15), 17);
  EXPECT_EQ(Compressors_pool::get_instance()("ppmd", ts3, 23), 21);
}

TEST_F(CustomCompressionMehtodsTest, Lcacomp) {
  // The sequences were compressed manually by the original ppmd program.
  EXPECT_EQ(Compressors_pool::get_instance()("lcacomp", ts1, 12), 16);
  EXPECT_EQ(Compressors_pool::get_instance()("lcacomp", ts2, 15), 28);
  EXPECT_EQ(Compressors_pool::get_instance()("lcacomp", ts3, 23), 32);
}

TEST_F(CustomCompressionMehtodsTest, Zstd) {
  // The sequences were compressed manually by the original ppmd program.
  EXPECT_EQ(Compressors_pool::get_instance()("zstd", ts1, 12), 16);
  EXPECT_EQ(Compressors_pool::get_instance()("zstd", ts2, 15), 24);
  EXPECT_EQ(Compressors_pool::get_instance()("zstd", ts3, 23), 32);
}

TEST_F(CustomCompressionMehtodsTest, Bzip2)
{
  // The sequences were compressed manually by the original ppmd program.
  EXPECT_EQ(Compressors_pool::get_instance()("bzip2", ts1, 12), 37);
  EXPECT_EQ(Compressors_pool::get_instance()("bzip2", ts2, 15), 43);
  EXPECT_EQ(Compressors_pool::get_instance()("bzip2", ts3, 23), 50);
}

TEST_F(CustomCompressionMehtodsTest, OneByOne) {
  EXPECT_EQ(Compressors_pool::get_instance()("bzip2", ts1, 12), 37);
  EXPECT_EQ(Compressors_pool::get_instance()("zstd", ts1, 12), 16);
  EXPECT_EQ(Compressors_pool::get_instance()("lcacomp", ts3, 23), 32);
}

TEST(RealPointwisePredictorTest, RealTsWithZeroDifferenceThreeStepsForecast_predict_PredictionIsCorrect) {
  PlainTimeSeries<Double> ts {3.4, 0.1, 3.9, 4.8, 1.5, 1.8, 2.0, 4.9, 5.1, 2.1};
  auto computer = std::make_shared<Codes_lengths_computer<Double>>();
  auto sampler = std::make_shared<Sampler<Double>>();
  auto partition_cardinality = 4u;
  auto horizont = 2u;
  std::vector<Names> compressors {{"zlib"}, {"rp"}, {"zlib", "rp"}};
  auto dpredictor = std::make_shared<Real_distribution_predictor<Double>> (computer, sampler, partition_cardinality);
  dpredictor->set_difference_order(0);
  Basic_pointwise_predictor<Double, Double> ppredictor{dpredictor};
  auto forecast = ppredictor.predict(ts, horizont, compressors);

  EXPECT_NEAR(forecast("zlib", 0).point, 1.8615389823, 1e-5);
  EXPECT_NEAR(forecast("zlib", 1).point, 1.8730772603, 1e-5);

  EXPECT_NEAR(forecast("rp", 0).point, 1.8896412464, 1e-5);
  EXPECT_NEAR(forecast("rp", 1).point, 1.8727279427, 1e-5);

  EXPECT_NEAR(forecast("zlib_rp", 0).point, 1.8896412464, 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 1).point, 1.8727279427, 1e-5);
}

TEST(DiscretePointwisePredictorTest, DiscreteTsWithZeroDifferenceTwoStepsForecast_predict_PredictionIsCorrect) {
  std::vector<unsigned char> ts {2, 0, 2, 3, 1, 1, 1, 3, 3, 1};
  auto computer = std::make_shared<Codes_lengths_computer<Symbol>>();
  auto sampler = std::make_shared<Sampler<Symbol>>();
  size_t horizont = 2u;
  std::vector<Names> compressors {{"zlib", "rp"}};
  auto dpredictor = std::make_shared<Discrete_distribution_predictor<Symbol>>(computer, sampler);
  Basic_pointwise_predictor<Symbol, Symbol> ppredictor {dpredictor};
  Forecast<Symbol> forecast = ppredictor.predict(ts, horizont, compressors);
  std::vector<double> expected_forecast{1.0264274976, 1.0151519618};
  EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
}

TEST(MultialphabetSparsePredictorTest, RealTsWithZeroDifferenceAndTwoPartitions_predict_PredictionIsCorrect) {
  std::vector<Double> ts {3.4, 0.1, 3.9, 4.8, 1.5, 1.8, 2.0, 4.9, 5.1, 2.1};
  auto computer = std::make_shared<Codes_lengths_computer<Double>>();
  auto sampler = std::make_shared<Sampler<Double>>();
  auto max_partition_cardinality = 4u;
  auto horizont = 2u;
  std::vector<Names> compressors {{"zlib", "rp"}};
  auto dpredictor = std::make_shared<Multialphabet_distribution_predictor<Double>>(computer, sampler,
                                                                                   max_partition_cardinality);
  dpredictor->set_difference_order(0);
  Basic_pointwise_predictor<Double, Double> ppredictor {dpredictor};
  Forecast<Double> forecast = ppredictor.predict(ts, horizont, compressors);
  std::vector<double> expected_forecast {3.0934987622, 3.0934080567};
  EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
}

TEST(SparseMultialphabetPredictorTest, RealTimeSeriesWithZeroDifference_predict_PredictionIsCorrect) {
  std::vector<Double> ts{3.4, 2.5, 0.1, 0.5, 3.9, 4.0, 4.8, 2.8, 1.5, 1.3, 1.8, 2.1,
        2, 3.5, 4.9, 5.0, 5.1, 4.5, 2.1};
  auto computer = std::make_shared<Codes_lengths_computer<Double>>();
  auto sampler = std::make_shared<Sampler<Double>>();
  size_t horizont {4};
  std::vector<Names> compressors {{"zlib", "rp"}};
  size_t max_quants_count = 4;
  auto dpredictor = std::make_shared<Multialphabet_distribution_predictor<Double>>(computer,
                                                                                   sampler,
                                                                                   max_quants_count);
  size_t sparse = 2;
  auto ppredictor = std::make_shared<Basic_pointwise_predictor<Double, Double>>(dpredictor);
  Sparse_predictor<Double, Double> sparse_predictor {ppredictor, sparse};
  Forecast<Double> forecast = sparse_predictor.predict(ts, horizont, compressors);
  std::vector<double> expected_forecast {3.7364683941, 3.8542121847, 3.0934080567, 2.75};
  EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 2).point, expected_forecast[2], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 3).point, expected_forecast[3], 1e-5);
}

TEST(SparseMultialphabetPredictorTest, SparseM3CYear_predict_PredictionIsCorrect) {
  PlainTimeSeries<Double> ts {940.66, 1084.86, 1244.98, 1445.02, 1683.17, 2038.15,
        2342.52, 2602.45, 2927.87, 3103.96, 3360.27, 3807.63, 4387.88, 4936.99};

  auto computer = std::make_shared<Codes_lengths_computer<Double>>();
  auto sampler = std::make_shared<Sampler<Double>>();
  size_t horizont = 6u;
  std::vector<Names> compressors {{"zlib", "rp"}};
  size_t max_quants_count = 4;
  auto dpredictor = std::make_shared<Multialphabet_distribution_predictor<Double>>(computer,
                                                                                   sampler,
                                                                                   max_quants_count);
  dpredictor->set_difference_order(1);
  auto ppredictor = std::make_shared<Basic_pointwise_predictor<Double, Double>>(dpredictor);
  size_t sparse = 2;
  Sparse_predictor<Double, Double> sparse_predictor{ppredictor, sparse};
  Forecast<Double> forecast = sparse_predictor.predict(ts, horizont, compressors);

  PlainTimeSeries<Double> expected_forecast {5427.0124308808, 5917.0363290153,
        6407.0594768841, 6262.0988838384, 6165.6275143097, 6999.809906989};
  EXPECT_NEAR(forecast("zlib_rp", 0).point, expected_forecast[0], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 1).point, expected_forecast[1], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 2).point, expected_forecast[2], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 3).point, expected_forecast[3], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 4).point, expected_forecast[4], 1e-5);
  EXPECT_NEAR(forecast("zlib_rp", 5).point, expected_forecast[5], 1e-5);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
