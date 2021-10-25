//
// Created by kon on 06.01.2020.
//

#include <selector.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "compressors_facade_mock.h"
#include "gtest_extensions.h"

using namespace itp;
using namespace itp::evaluation;
using namespace testing;

class DiffTest : public Test
{
protected:
	std::vector<int> test_vec_ = {1, 2, 7, 4};
};

TEST_F(DiffTest, ReturnsOriginalSeriesIfDifferenceOrderIsZero)
{
	EXPECT_THAT(diff_n(test_vec_, 0), ElementsAreArray(test_vec_));
}

TEST_F(DiffTest, CorrectlyComputesFirstDifference)
{
	EXPECT_THAT(diff_n(test_vec_, 1), ElementsAre(1, 5, -3));
}

TEST_F(DiffTest, CorrectlyComputesSecondDifference)
{
	EXPECT_THAT(diff_n(test_vec_, 2), ElementsAre(4, -8));
}

TEST_F(DiffTest, ReturnsEmptySequenceIfDiffOrderEqualsToNumberOfElements)
{
	EXPECT_THAT(diff_n(test_vec_, 4), IsEmpty());
}

TEST_F(DiffTest, CorrectlyComputesDifferenceUpUntilSingleElement)
{
	EXPECT_THAT(diff_n(test_vec_, 3), ElementsAre(-12));
}

TEST_F(DiffTest, ReturnsEmptySequenceIfDiffOrderIsGreaterThanoNumberOfElements)
{
	EXPECT_THAT(diff_n(test_vec_, 5), IsEmpty());
}

TEST(SampledSeriesStorageTest, WorksInDiscreteCase)
{
	std::vector<Symbol> ts{9, 1, 3};
	SampledSeriesStorage<Symbol> storage{ts};

	auto res = storage.cbegin();
	EXPECT_THAT(*res, ElementsAre(8., 0., 2.));
	EXPECT_EQ(res->GetAlphabetSize(), 9);
}

TEST(SampledSeriesStorageTest, WorksInRealCase)
{
	std::vector<Double> ts{4.2, 5.6, 7.8, 1.4};
	std::vector<size_t> quanta_count{2, 4};
	SampledSeriesStorage<Double> storage{ts, quanta_count};

	std::vector<std::vector<Symbol>> expected_series =
    {
            {0, 1, 1, 0},
            {1, 2, 3, 0}
    };

	ExpectContainerRangesEq(storage, expected_series);
    ExpectTransformedContainerEq(quanta_count, storage, [](const auto& item) { return item.GetAlphabetSize(); });
}

TEST(SampledSeriesStorageTest, QuantedSeriesContainRightQuantaCount)
{
    const std::vector<size_t> quanta_count{2, 4};

    const std::vector<Double> ts{4.2, 5.6, 7.8, 1.4};
    const SampledSeriesStorage<Double> storage{ts, quanta_count};
}

TEST(ComputeCorrectionsTest, ReturnsZeroOnEmptyInputInDiscreteCase)
{
	EXPECT_THAT(ComputeCorrections<Symbol>({}, 10), ElementsAre(0));
}

TEST(ComputeCorrectionsTest, ReturnsAlwaysZeroInDescreteOnDimensionalCase)
{
	EXPECT_THAT(ComputeCorrections<Symbol>({2, 4, 8}, 10), ElementsAre(0));
}

TEST(ComputeCorrectionsTest, ReturnsAlwaysZeroInDescreteOnMultidimensionalCase)
{
	EXPECT_THAT(ComputeCorrections<VectorSymbol>({2, 4, 8}, 10), ElementsAre(0));
}

TEST(ComputeCorrectionsTest, ReturnsEmptyVectorOnEmptyInputInRealCase)
{
	EXPECT_THAT(ComputeCorrections<Double>({}, 10), IsEmpty());
}

TEST(ComputeCorrectionsTest, WorksInRealCase)
{
	EXPECT_THAT(ComputeCorrections<Double>({8, 2, 4}, 10), ElementsAre(0, 10 * 2, 10));
}

class SelectorDiscreteCaseTest : public Test
{
protected:
	SelectorDiscreteCaseTest();

	CompressorsFacadeMock *compressors_;
	std::unique_ptr<CodeLengthEvaluator<unsigned char>> evaluator_;
	std::vector<unsigned char> test_discrete_ts_;
};

SelectorDiscreteCaseTest::SelectorDiscreteCaseTest()
{
	auto compressors = std::make_unique<CompressorsFacadeMock>();
	compressors_ = compressors.get();
	evaluator_ = std::make_unique<CodeLengthEvaluator<unsigned char>>(std::move(compressors));
	test_discrete_ts_= {2, 5, 4};
}

TEST_F(SelectorDiscreteCaseTest, ReturnsZeroIfInputSeriesIsEmpty)
{
	auto result = evaluator_->Evaluate({}, {"zlib"}, 0, {2});
	EXPECT_EQ(result.at("zlib"), 0);
}

TEST_F(SelectorDiscreteCaseTest, ReturnsZeroIfAllItemsAreConsumedByDifferentiation)
{
	auto difference = test_discrete_ts_.size() + 1;
	auto result = evaluator_->Evaluate(test_discrete_ts_, {"zlib"}, difference, {2});
	EXPECT_EQ(result.at("zlib"), 0);
}

TEST_F(SelectorDiscreteCaseTest, CallsCompressorWithNormalizedDiscreteSeriesIgnoringQuantaCounts)
{
	std::vector<unsigned char> expected_time_series{0, 3, 2};
	EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _,
			expected_time_series.size())).With(Args<1, 2>(ElementsAreArray(expected_time_series))).Times(1).WillOnce(Return(0));
	evaluator_->Evaluate(test_discrete_ts_, {"zlib"}, 0, {2, 4});
}

TEST_F(SelectorDiscreteCaseTest, CallsCompressorWithNormalizedDiscreteSeriesEvenIfQuantaCountsIsEmpty)
{
	std::vector<unsigned char> expected_time_series{0, 3, 2};
	EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _,
										expected_time_series.size())).With(Args<1, 2>(ElementsAreArray(expected_time_series))).Times(1).WillOnce(Return(0));
	evaluator_->Evaluate(test_discrete_ts_, {"zlib"}, 0, {});
}

class SelectorRealCaseTest : public Test
{
protected:
	SelectorRealCaseTest();

	CompressorsFacadeMock *compressors_;
	std::unique_ptr<CodeLengthEvaluator<Double>> evaluator_;
	std::vector<Double> test_real_ts_;
};

SelectorRealCaseTest::SelectorRealCaseTest()
{
	auto compressors = std::make_unique<CompressorsFacadeMock>();
	compressors_ = compressors.get();
	evaluator_ = std::make_unique<CodeLengthEvaluator<Double>>(std::move(compressors));
	test_real_ts_ = {4.2, 5.6, 7.8, 1.4};
}

TEST_F(SelectorRealCaseTest, ReturnsZeroIfInputSeriesIsEmpty)
{
	auto result = evaluator_->Evaluate({}, {"zlib"}, 0, {2});
	EXPECT_EQ(result.at("zlib"), 0);
}

TEST_F(SelectorRealCaseTest, CallsCompressorWithSeriesDiscretizedUsingAllSpecifiedQuantaCounts)
{
	{
		InSequence s;
		EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _, test_real_ts_.size())).With(Args<1, 2>(ElementsAre(0, 1, 1, 0))).Times(1).WillOnce(Return(0));
		EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _, test_real_ts_.size())).With(Args<1, 2>(ElementsAre(1, 2, 3, 0))).Times(1).WillOnce(Return(0));
	}

	evaluator_->Evaluate(test_real_ts_, {"zlib"}, 0, {2, 4});
}

MATCHER_P(AlphabetsEqual, other,"")
{
	return std::tie(arg.min_symbol, arg.max_symbol) == std::tie(other.min_symbol, other.max_symbol);
}

TEST_F(SelectorRealCaseTest, SetsEachQuantaCountOnEvaluator)
{
	auto compressors_pool = std::make_unique<NiceMock<CompressorsFacadeMock>>();
	EXPECT_CALL(*compressors_pool, SetAlphabetDescription(AlphabetsEqual(AlphabetDescription{0, 1})));
	EXPECT_CALL(*compressors_pool, SetAlphabetDescription(AlphabetsEqual(AlphabetDescription{0, 3})));

	evaluator_ = std::make_unique<CodeLengthEvaluator<Double>>(std::move(compressors_pool));
	evaluator_->Evaluate(test_real_ts_, {"zlib"}, 0, {2, 4});
}

TEST_F(SelectorRealCaseTest, ThrowsIfNoQuantaCountIsSpecified)
{
	EXPECT_THROW(evaluator_->Evaluate(test_real_ts_, {"zlib", "ppmd"}, 0, {}), SelectorError);
}

TEST_F(SelectorRealCaseTest, ReturnsCodeLengthForEachCompressor)
{
	EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _, _)).Times(2).WillOnce(Return(25)).WillOnce(Return(30));
	EXPECT_CALL(*compressors_, Compress(Eq("ppmd"), _, _)).Times(2).WillOnce(Return(40)).WillOnce(Return(25));

	const auto result = evaluator_->Evaluate(test_real_ts_, {"zlib", "ppmd"}, 0, {2, 4});

	// We must add 4 bits because log_2 4 - log_2 2 = 1 and test_real_ts_.size() == 4.
	EXPECT_EQ(result.at("zlib"), 29);
	EXPECT_EQ(result.at("ppmd"), 25);
}

TEST_F(SelectorRealCaseTest, ReturnsEmptyMapIfNoCompressorsSpecified)
{
	EXPECT_THAT(evaluator_->Evaluate(test_real_ts_, {}, 0, {2, 4}), IsEmpty());
}

TEST_F(SelectorRealCaseTest, SelectsMaxPartitionInCaseOfEqualCodeLengths)
{
	{
		InSequence s;
		EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _, _)).Times(4).WillRepeatedly(Return(100));
	}

	auto result = evaluator_->Evaluate(test_real_ts_, {"zlib"}, 0, {4, 2, 16, 8});
	EXPECT_EQ(result["zlib"], 100);
}

TEST_F(SelectorRealCaseTest, PaysAttentionToReducedSeriesLengthsAfterDifferentiation)
{
	{
		InSequence s;
		EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _, _)).Times(1).WillOnce(Return(25));
		EXPECT_CALL(*compressors_, Compress(Eq("zlib"), _, _)).Times(1).WillOnce(Return(30));
	}

	auto result = evaluator_->Evaluate(test_real_ts_, {"zlib"}, 1, {2, 4});
	EXPECT_EQ(result["zlib"], 28);
}

TEST(SelectBestCompressorsTest, ThrowsIfResultsOfComputationsAreEmpty)
{
	EXPECT_THROW(GetBestCompressors({}, 1), SelectorError);
}

TEST(SelectBestCompressorsTest, ThrowsIfTargetNumberOfCompressorsIsMoreThanInResultsOfComputations)
{
	EXPECT_THROW(GetBestCompressors({{"zlib", 10},
									 {"ppmd", 20}}, 3), SelectorError);
}

TEST(SelectBestCompressorsTest, TargetNumberOfCompressorsCanBeEqualToTheSizeOfResultsOfComputations)
{
	EXPECT_THAT(GetBestCompressors({{"zlib", 10},
									{"ppmd", 20}}, 2), UnorderedElementsAre("zlib", "ppmd"));
}

TEST(SelectBestCompressorsTest, SelectsSpecefiedNumberOfCompressorsWithMinimalCodeLengths)
{
	EXPECT_THAT(GetBestCompressors({{"zlib",  10},
									{"ppmd",  20},
									{"bzip2", 15}}, 2), UnorderedElementsAre("zlib", "bzip2"));
}

TEST(ShareTest, AllowsExplicitInitializationFromDouble)
{
	EXPECT_NO_THROW(Share{0.1});
}

TEST(ShareTest, ThrowsIfShareIsNegative)
{
	EXPECT_THROW(Share{-0.1}, std::invalid_argument);
}

TEST(ShareTest, ThrowsIfShareIsGreaterThanOne)
{
	EXPECT_THROW(Share{1.1}, std::invalid_argument);
}

TEST(ShareTest, AllowsImplicitCastingToDouble)
{
	double double_share = Share{0.2};
	EXPECT_DOUBLE_EQ(double_share, 0.2);
}
