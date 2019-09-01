#include <sampler.h>

#include <algorithm>
#include "itp_exceptions.h"
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

template <typename T>
void ExpectValarrayEmpty(const std::valarray<T> &va) {
  EXPECT_EQ(va.size(), 0);
}

template <typename T>
void ExpectValarrayEq(const std::valarray<T> &lhs, const std::valarray<T> &rhs) {
  EXPECT_EQ(lhs.size(), rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    EXPECT_EQ(lhs[i], rhs[i]);
  }
}

template <>
void ExpectValarrayEq(const std::valarray<Double> &lhs, const std::valarray<Double> &rhs) {
  EXPECT_EQ(lhs.size(), rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    EXPECT_DOUBLE_EQ(lhs[i], rhs[i]);
  }
}

TEST_F(SamplerForVectorDoublesTest, InverseTransformationWorks) {
  auto sampled_ts = sampler_.Transform({{0.4, 1.4}, {1.2, 1.6}, {0.6, 1.4}, {1.8, 1.8}}, kCountOfIntervalsToSplit);
  ExpectValarrayEq(sampler_.InverseTransform(0, sampled_ts), {0.68, 1.48});
  ExpectValarrayEq(sampler_.InverseTransform(1, sampled_ts), {1.52, 1.48});
  ExpectValarrayEq(sampler_.InverseTransform(2, sampled_ts), {0.68, 1.72});
  ExpectValarrayEq(sampler_.InverseTransform(3, sampled_ts), {1.52, 1.72});
}

TEST_F(SamplerForVectorDoublesTest, ThrowsIfInputNumberIsOutOfRange) {
  auto sampled_ts = sampler_.Transform({{0.4, 1.4}, {1.2, 1.6}, {0.6, 1.4}, {1.8, 1.8}}, kCountOfIntervalsToSplit);
  EXPECT_THROW(sampler_.InverseTransform(4, sampled_ts), RangeError);
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
  ExpectValarrayEmpty(pointwise_min_elements(std::begin(empty_vec), std::end(empty_vec)));
}

TEST(PointwiseMinElementsTest, CorrectlyWorksOnValidData) {
  std::vector<VectorSymbol> vec {{4, 2}, {1, 3}};
  ExpectValarrayEq(pointwise_min_elements(std::begin(vec), std::end(vec)), {1, 2});
}

TEST(PointwiseMaxElementsTest, CorrectlyWorksOnValidData) {
  std::vector<VectorSymbol> vec {{4, 2}, {1, 3}};
  ExpectValarrayEq(pointwise_max_elements(std::begin(vec), std::end(vec)), {4, 3});
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
  ExpectValarrayEq(ConvertDecToNumber(0, 2), {0});
  ExpectValarrayEq(ConvertDecToNumber(0, 5), {0});
}

TEST(ConvertDecToNumberTest, ThrowsOnInvalidBase) {
  EXPECT_THROW(ConvertDecToNumber(0, 0), InvalidBaseError);
  EXPECT_THROW(ConvertDecToNumber(0, 1), InvalidBaseError);
}

TEST(ConvertDecToNumberTest, ConvertsNumbersWithValidBase) {
  ExpectValarrayEq(ConvertDecToNumber(2, 2), {0, 1});
  ExpectValarrayEq(ConvertDecToNumber(15, 3), {0, 2, 1});
}
