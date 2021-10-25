#include <gtest/gtest.h>

#include "NonCompressionAlgorithmMock.h"

#include "../src/Macro.h"
#include "../src/NonCompressionAlgorithmAdaptor.h"

#include <algorithm>

using namespace itp;
using namespace testing;

class NonCompressionAlgorithmAdaptorTest : public ::testing::Test
{
protected:
	NonCompressionAlgorithmAdaptorTest()
		: out_buffer_(10)
	{
		algorithm_ = std::make_unique<NiceMock<mocks::NonCompressionAlgorithmMock>>();
		adaptor_ = std::make_unique<NonCompressionAlgorithmAdaptor>(algorithm_.get());
	}

	std::pair<Symbol, ConfidenceLevel> Prediction(const Symbol symbol, const ConfidenceLevel confidence_level)
	{
		return {symbol, confidence_level};
	}

	std::unique_ptr<NiceMock<mocks::NonCompressionAlgorithmMock>> algorithm_;
	std::unique_ptr<NonCompressionAlgorithmAdaptor> adaptor_;

	const unsigned char data_[7] = {1, 2, 1, 1, 2, 1, 1};
	static constexpr size_t size_ = ARRAY_SIZE(data_);
	std::vector<unsigned char> out_buffer_;
};

TEST_F(NonCompressionAlgorithmAdaptorTest, ForwardsAlphabetInformation)
{
	const auto [min, max] = std::minmax_element(std::cbegin(data_), std::cend(data_));
	EXPECT_CALL(*algorithm_, SetTsParams(*min, *max));
	adaptor_->SetTsParams(*min, *max);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, CorrectlyEvaluatesCodeLength)
{
	const auto [min, max] = std::minmax_element(std::cbegin(data_), std::cend(data_));
	adaptor_->SetTsParams(*min, *max);

	const std::array expected_return_values = {
		Prediction(1, ConfidenceLevel::kNotConfident),
		Prediction(1, ConfidenceLevel::kNotConfident),
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(2, ConfidenceLevel::kConfident),
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(1, ConfidenceLevel::kConfident)
	};

	testing::InSequence seq;
	for (size_t i = 0; i < std::size(expected_return_values); ++i)
	{
		EXPECT_CALL(*algorithm_, GiveNextPrediction(data_, i)).WillOnce(Return(expected_return_values[i]));
	}

	const auto expected_result = static_cast<size_t>(std::ceil(-std::log2(7.0*9.0*11.0/2.0/4.0/4.0/6.0/8.0/2.0/4.0)));
	EXPECT_EQ(adaptor_->Compress(data_, size_, &out_buffer_), expected_result);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, ReturnsZeroForEmptyInput)
{
	EXPECT_EQ(adaptor_->Compress(data_, 0, &out_buffer_), 0);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, SignalsAboutErrorOnNullptrAsData)
{
	EXPECT_THROW(adaptor_->Compress(nullptr, size_, &out_buffer_), std::runtime_error);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, AutomaticallyComputesUnsepcifiedMinAndMaxValues)
{
	EXPECT_CALL((*algorithm_), SetTsParams(1, 2));
	adaptor_->Compress(data_, size_, &out_buffer_);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, NonConfidentPredictionResetsSeriesOfConfidentPredictions)
{
	const auto [min, max] = std::minmax_element(std::cbegin(data_), std::cend(data_));
	adaptor_->SetTsParams(*min, *max);

	const std::array expected_return_values = {
		Prediction(1, ConfidenceLevel::kNotConfident),
		Prediction(1, ConfidenceLevel::kNotConfident),
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(2, ConfidenceLevel::kNotConfident),
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(1, ConfidenceLevel::kConfident)
	};

	testing::InSequence seq;
	for (size_t i = 0; i < std::size(expected_return_values); ++i)
	{
		EXPECT_CALL(*algorithm_, GiveNextPrediction(data_, i)).WillOnce(Return(expected_return_values[i]));
	}

	const auto expected_result = static_cast<size_t>(std::ceil(-std::log2(3.0*5.0*3.0*3.0*5.0/2.0/4.0/4.0/6.0/10.0/4.0/6.0)));
	EXPECT_EQ(adaptor_->Compress(data_, size_, &out_buffer_), expected_result);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, InNonConfidentPredictionCaseAlgorithmCountsAllPreviousSymbols)
{
	const unsigned char test_data[] = {1, 1, 1};
	const size_t size = ARRAY_SIZE(test_data);
	adaptor_->SetTsParams(1, 2);

	const std::array expected_return_values = {
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(1, ConfidenceLevel::kConfident),
		Prediction(1, ConfidenceLevel::kNotConfident)
	};

	testing::InSequence seq;
	for (size_t i = 0; i < std::size(expected_return_values); ++i)
	{
		EXPECT_CALL(*algorithm_, GiveNextPrediction(test_data, i)).WillOnce(Return(expected_return_values[i]));
	}

	const auto expected_result = static_cast<size_t>(std::ceil(-std::log2(3.0*5.0*5.0/4.0/6.0/6.0)));
	EXPECT_EQ(adaptor_->Compress(test_data, size, &out_buffer_), expected_result);
}

class GiveNextPredictionCallsChecker : public INonCompressionAlgorithm
{
public:
	explicit GiveNextPredictionCallsChecker(std::vector<std::vector<Symbol>> expected_data_contents);

	Guess GiveNextPrediction(const unsigned char* data, size_t size) override;
	void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) override;

	bool AllCallsAreAsExpected() const;
	std::string GetErrorsDescription() const;

private:
	struct ExpectedCall
	{
		std::vector<Symbol> data_content;
		bool expired = false;
	};

	static bool AreSame(const std::vector<Symbol>& data_content, const unsigned char* data, size_t size);

	Symbol dumb_guess_;

	std::vector<ExpectedCall> expectations_;
	std::ostringstream log_;
};

GiveNextPredictionCallsChecker::GiveNextPredictionCallsChecker(
	std::vector<std::vector<Symbol>> expected_data_contents)
	: expectations_{std::size(expected_data_contents)}
{
	for (size_t i = 0; i < std::size(expected_data_contents); ++i)
	{
		expectations_[i].data_content = std::move(expected_data_contents[i]);
	}
}

GiveNextPredictionCallsChecker::Guess GiveNextPredictionCallsChecker::GiveNextPrediction(
	const unsigned char* data,
	size_t size)
{
	size_t expired_occurrences_count = 0;
	for (auto& [data_content, expired] : expectations_)
	{
		if (AreSame(data_content, data, size))
		{
			if (expired)
			{
				++expired_occurrences_count;
				continue;
			}

			expired = true;
			return {dumb_guess_, ConfidenceLevel::kConfident};
		}
	}

	if (expired_occurrences_count)
	{
		log_ << "Expected " << expired_occurrences_count << " times, but encountered again: ";
	}
	else
	{
		log_ << "Unexpected data: ";
	}

	for (size_t i = 0; i < size; ++i)
	{
		log_ << static_cast<int>(data[i]) << ' ';
	}
	log_ << '\n';

	return {dumb_guess_, ConfidenceLevel::kConfident};
}

void GiveNextPredictionCallsChecker::SetTsParams(Symbol alphabet_min_symbol, Symbol)
{
	dumb_guess_ = alphabet_min_symbol;
}

bool GiveNextPredictionCallsChecker::AllCallsAreAsExpected() const
{
	return std::size(log_.str()) == 0;
}

std::string GiveNextPredictionCallsChecker::GetErrorsDescription() const
{
	return log_.str();
}

bool GiveNextPredictionCallsChecker::AreSame(
	const std::vector<Symbol>& data_content,
	const unsigned char* data,
	size_t size)
{
	if (data_content.size() != size)
	{
		return false;
	}

	return std::equal(std::cbegin(data_content), std::cend(data_content), data);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, RequestsPredictionsPassingRightData)
{
	const std::vector<Symbol> test_data = {0, 1, 0};
	const std::vector<Continuation<Symbol>> continuations = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

	auto algorithm = GiveNextPredictionCallsChecker(
		{
			{},
			{0},
			{0, 1},
			{0, 1, 0},
			{0, 1, 0, 0},
			{0, 1, 0},
			{0, 1, 0, 0},
			{0, 1, 0},
			{0, 1, 0, 1},
			{0, 1, 0},
			{0, 1, 0, 1},
		});

	auto adaptor = std::make_unique<NonCompressionAlgorithmAdaptor>(&algorithm);
	adaptor->SetTsParams(0, 1);

	adaptor->CompressEndings(test_data, continuations);

	EXPECT_TRUE(algorithm.AllCallsAreAsExpected()) << algorithm.GetErrorsDescription();
}
