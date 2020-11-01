#include <gtest/gtest.h>

#include "NonCompressionAlgorithmMock.h"

#include <Macro.h>
#include <NonCompressionAlgorithmAdaptor.h>

#include <algorithm>

using namespace itp;
using namespace testing;

class NonCompressionAlgorithmAdaptorTest : public ::testing::Test
{
protected:
	NonCompressionAlgorithmAdaptorTest()
		: out_buffer_(10)
	{
		auto algorithm_ptr = std::make_unique<NiceMock<mocks::NonCompressionAlgorithmMock>>();
		algorithm_ = algorithm_ptr.get();
		adaptor_ = std::make_unique<NonCompressionAlgorithmAdaptor>(std::move(algorithm_ptr));
	}

	std::pair<Symbol, ConfidenceLevel> Prediction(const Symbol symbol, const ConfidenceLevel confidence_level)
	{
		return {symbol, confidence_level};
	}

	NiceMock<mocks::NonCompressionAlgorithmMock>* algorithm_;
	std::unique_ptr<NonCompressionAlgorithmAdaptor> adaptor_;

	const unsigned char data_[7] = {1, 2, 1, 1, 2, 1, 1};
	static constexpr size_t size_ = ARRAY_SIZE(data_);
	std::vector<unsigned char> out_buffer_;
};

TEST_F(NonCompressionAlgorithmAdaptorTest, ForwardsFullTimeSeries)
{
	// In order to avoid forecasting process.
	size_t size = 0;
	EXPECT_CALL(*algorithm_, RegisterFullTimeSeries(data_, size));
	(*adaptor_)(data_, size, &out_buffer_);
}

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

	EXPECT_CALL(*algorithm_, GiveNextPrediction()).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kNotConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kNotConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(2, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident)));
	const auto expected_result = static_cast<size_t>(std::ceil(-std::log2(7.0*9.0*11.0/2.0/4.0/4.0/6.0/8.0/2.0/4.0)));
	EXPECT_EQ((*adaptor_)(data_, size_, &out_buffer_), expected_result);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, ReturnsZeroForEmptyInput)
{
	EXPECT_EQ((*adaptor_)(data_, 0, &out_buffer_), 0);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, SignalsAboutErrorOnNullptrAsData)
{
	EXPECT_THROW((*adaptor_)(nullptr, size_, &out_buffer_), std::runtime_error);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, AutomaticallyComputesUnsepcifiedMinAndMaxValues)
{
	EXPECT_CALL((*algorithm_), SetTsParams(1, 2));
	(*adaptor_)(data_, size_, &out_buffer_);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, NonConfidentPredictionResetsSeriesOfConfidentPredictions)
{
	const auto [min, max] = std::minmax_element(std::cbegin(data_), std::cend(data_));
	adaptor_->SetTsParams(*min, *max);

	EXPECT_CALL((*algorithm_), GiveNextPrediction()).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kNotConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kNotConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(2, ConfidenceLevel::kNotConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident)));
	const auto expected_result = static_cast<size_t>(std::ceil(-std::log2(3.0*5.0*3.0*3.0*5.0/2.0/4.0/4.0/6.0/10.0/4.0/6.0)));
	EXPECT_EQ((*adaptor_)(data_, size_, &out_buffer_), expected_result);
}

TEST_F(NonCompressionAlgorithmAdaptorTest, InNonConfidentPredictionCaseAlgorithmCountsAllPreviousSymbols)
{
	const unsigned char test_data[] = {1, 1, 1};
	const size_t size = ARRAY_SIZE(test_data);
	adaptor_->SetTsParams(1, 2);

	EXPECT_CALL((*algorithm_), GiveNextPrediction()).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kConfident))).
		WillOnce(Return(Prediction(1, ConfidenceLevel::kNotConfident)));
	const auto expected_result = static_cast<size_t>(std::ceil(-std::log2(3.0*5.0*5.0/4.0/6.0/6.0)));
	EXPECT_EQ((*adaptor_)(test_data, size, &out_buffer_), expected_result);
}
