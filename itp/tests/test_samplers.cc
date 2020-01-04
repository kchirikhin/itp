#include <sampler.h>

#include <algorithm>
#include <itp_exceptions.h>
#include <tseries.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "gtest_extensions.h"

using namespace itp;
using namespace testing;

class SamplerForDoublesTest : public Test {
 protected:
  static constexpr size_t kCountOfIntervalsToSplit = 2;

  Sampler<Double> sampler_;
};

TEST_F(SamplerForDoublesTest, QuantizesEmptySeries) {
  EXPECT_THAT(sampler_.Transform({}, kCountOfIntervalsToSplit), IsEmpty());
}

TEST_F(SamplerForDoublesTest, ThrowsOnASeriesWithSingleValue) {
  EXPECT_THROW(sampler_.Transform({1.}, kCountOfIntervalsToSplit), SeriesTooShortError);
}

TEST_F(SamplerForDoublesTest, QuantizesASeriesWithNegativeValues) {
  EXPECT_THAT(sampler_.Transform({0.1, -0.5, 5, -1, 7.5}, kCountOfIntervalsToSplit), ElementsAre(0, 0, 1, 0, 1));
}

TEST_F(SamplerForDoublesTest, CorrectlyDequantizesASeries) {
  std::vector<Double> expected_series { 0.7, 0.7, 5.8, 0.7, 5.8 };

  auto transformed_series = sampler_.Transform({0.1, -0.5, 5, -1, 7.5}, kCountOfIntervalsToSplit);
  ASSERT_EQ(transformed_series.size(), expected_series.size());
  for (size_t i = 0; i < transformed_series.size(); ++i) {
    EXPECT_DOUBLE_EQ(sampler_.InverseTransform(transformed_series[i], transformed_series), expected_series[i]);
  }
}

class SamplerForIntegersTest : public Test {
 protected:
  Sampler<Symbol> sampler_;
};

TEST_F(SamplerForIntegersTest, TransformsEmptySeries) {
  EXPECT_THAT(sampler_.Transform({}), IsEmpty());
}

TEST_F(SamplerForIntegersTest, TransformsASeriesWithSingleValueToTheSeriesWithZero) {
  EXPECT_THAT(sampler_.Transform({5}), ElementsAre(0));
}

TEST_F(SamplerForIntegersTest, TransformsASeriesWithNonNegativeValues) {
  EXPECT_THAT(sampler_.Transform({1, 5, 2, 4, 2}), ElementsAre(0, 4, 1, 3, 1));
}

TEST_F(SamplerForIntegersTest, InverseTransformForNonNegativeIntegersWorks) {
  std::vector<Symbol> test_series {1, 5, 2, 4, 2};

  auto transformed_series = sampler_.Transform(test_series);
  ASSERT_EQ(transformed_series.size(), test_series.size());
  for (size_t i = 0; i < transformed_series.size(); ++i) {
    EXPECT_DOUBLE_EQ(sampler_.InverseTransform(transformed_series[i], transformed_series), test_series[i]);
  }
}

class SamplerForVectorDoublesTest : public Test {
 protected:
  static constexpr size_t kCountOfIntervalsToSplit = 2;
  
  Sampler<VectorDouble> sampler_;
};

TEST_F(SamplerForVectorDoublesTest, ReturnsEmptySeriesIfInputSeriesIsEmpty) {
  EXPECT_THAT(sampler_.Transform({}, kCountOfIntervalsToSplit), IsEmpty());
}

TEST_F(SamplerForVectorDoublesTest, ThrowsOnASeriesWithSingleValue) {
  EXPECT_THROW(sampler_.Transform({{1., 2.}}, kCountOfIntervalsToSplit), SeriesTooShortError);
}

TEST_F(SamplerForVectorDoublesTest, QuantizesTwoSeriesWithPositiveValues) {
  EXPECT_THAT(sampler_.Transform({{0.4, 1.4}, {1.2, 1.6}, {0.6, 1.4}, {1.8, 1.8}}, kCountOfIntervalsToSplit),
              ElementsAre(0, 3, 0, 3));
}

TEST_F(SamplerForVectorDoublesTest, InverseTransformationWorks) {
  auto sampled_ts = sampler_.Transform({{0.4, 1.4}, {1.2, 1.6}, {0.6, 1.4}, {1.8, 1.8}}, kCountOfIntervalsToSplit);
  ExpectDoubleContainersEq(sampler_.InverseTransform(0, sampled_ts), VectorDouble{0.68, 1.48});
  ExpectDoubleContainersEq(sampler_.InverseTransform(1, sampled_ts), VectorDouble{1.52, 1.48});
  ExpectDoubleContainersEq(sampler_.InverseTransform(2, sampled_ts), VectorDouble{0.68, 1.72});
  ExpectDoubleContainersEq(sampler_.InverseTransform(3, sampled_ts), VectorDouble{1.52, 1.72});
}

TEST_F(SamplerForVectorDoublesTest, InverseTransformationWorksWithMoreThanTwoSubintervals) {
	auto sampled_ts = sampler_.Transform({{0.4, 1.4}, {1.2, 1.6}, {0.6, 1.4}, {1.8, 1.8}}, 8);
	ExpectDoubleContainersEq(sampler_.InverseTransform(0, sampled_ts), VectorDouble{0.365, 1.39});
}

TEST_F(SamplerForVectorDoublesTest, ThrowsIfInputNumberIsOutOfRange) {
  auto sampled_ts = sampler_.Transform({{0.4, 1.4}, {1.2, 1.6}, {0.6, 1.4}, {1.8, 1.8}}, kCountOfIntervalsToSplit);
  EXPECT_THROW(sampler_.InverseTransform(4, sampled_ts), RangeError);
}

TEST_F(SamplerForVectorDoublesTest, ThrowsIfSizeOfAlphabetExceeds256) {
  // count_of_series^count_of_intervals intervals after transformation!
  const size_t kLargeIntervalsCount = 64;
  EXPECT_THROW(sampler_.Transform({{0.4, 1.4}, {1.2, 1.6}, {0.6, 1.4}, {1.8, 1.8}}, kLargeIntervalsCount),
               IntervalsCountError);
}

class SamplerForVectorSymbolsTest : public Test {
 protected:
  Sampler<VectorSymbol> sampler_;
};

TEST_F(SamplerForVectorSymbolsTest, DISABLED_ReturnsEmptySeriesIfInputSeriesIsEmpty) {
  EXPECT_THAT(sampler_.Transform({}), IsEmpty());
}

TEST_F(SamplerForVectorSymbolsTest, DISABLED_TransformsASeriesWithSoleValue) {
  EXPECT_THAT(sampler_.Transform({{1, 2}}), ElementsAre(0, 0));
}

TEST(PointwiseMinElementsTest, ReturnsEmptyVectorOnEmptyInput) {
  std::vector<VectorSymbol> empty_vec {{}};
  ExpectContainerEmpty(pointwise_min_elements(std::begin(empty_vec), std::end(empty_vec)));
}

TEST(PointwiseMinElementsTest, CorrectlyWorksOnValidData) {
  std::vector<VectorSymbol> vec {{4, 2}, {1, 3}};
  ExpectContainersEq(pointwise_min_elements(std::begin(vec), std::end(vec)), std::vector<Symbol>{1, 2});
}

TEST(PointwiseMaxElementsTest, CorrectlyWorksOnValidData) {
  std::vector<VectorSymbol> vec {{4, 2}, {1, 3}};
  ExpectContainersEq(pointwise_max_elements(std::begin(vec), std::end(vec)), std::vector<Symbol>{4, 3});
}

TEST(ConvertNumberToDecTest, ThrowsOnEmptyInput) {
  EXPECT_THROW(ConvertNumberToDec({}, 2), EmptyInputError);
}

TEST(ConvertNumberToDecTest, ConvertsSingleDigit) {
  EXPECT_EQ(ConvertNumberToDec({0}, 2), 0);
  EXPECT_EQ(ConvertNumberToDec({1}, 2), 1);
}

TEST(ConvertNumberToDecTest, ThrowsWhenADigitGeThanTheBase) {
  EXPECT_THROW(ConvertNumberToDec({2}, 2), InvalidDigitError);
  EXPECT_THROW(ConvertNumberToDec({3}, 2), InvalidDigitError);
  EXPECT_THROW(ConvertNumberToDec({1, 1, 3}, 2), InvalidDigitError);
}

TEST(ConvertNumberToDecTest, ThrowsOnInvalidBase) {
  EXPECT_THROW(ConvertNumberToDec({0}, 0), InvalidBaseError);
  EXPECT_THROW(ConvertNumberToDec({0}, 1), InvalidBaseError);
}

TEST(ConvertNumberToDecTest, ConvertsNumbersWithMoreThanOneDigit) {
  EXPECT_EQ(ConvertNumberToDec({0, 1}, 2), 2);
  EXPECT_EQ(ConvertNumberToDec({2, 2}, 3), 8);
  EXPECT_EQ(ConvertNumberToDec({2, 2, 1}, 4), 26);
}

TEST(ConvertDecToNumberTest, ConvertsZeroToVectorWithASoleZeroForAnyBase) {
  ExpectContainersEq(ConvertDecToNumber(0, 2), VectorSymbol{0});
  ExpectContainersEq(ConvertDecToNumber(0, 5), VectorSymbol{0});
}

TEST(ConvertDecToNumberTest, ThrowsOnInvalidBase) {
  EXPECT_THROW(ConvertDecToNumber(0, 0), InvalidBaseError);
  EXPECT_THROW(ConvertDecToNumber(0, 1), InvalidBaseError);
}

TEST(ConvertDecToNumberTest, ConvertsNumbersWithValidBase) {
  ExpectContainersEq(ConvertDecToNumber(2, 2), VectorSymbol{0, 1});
  ExpectContainersEq(ConvertDecToNumber(15, 3), VectorSymbol{0, 2, 1});
}

