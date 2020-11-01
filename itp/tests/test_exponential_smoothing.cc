#include <gtest/gtest.h>

#include <Macro.h>
#include <PythonCompressor.h>

#include <optional>

using namespace itp;


namespace
{

void SkipPredictions(INonCompressionAlgorithm& algorithm, const size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		const auto [prediction, confidence] = algorithm.GiveNextPrediction();
		UNUSED(prediction);
		UNUSED(confidence);
	}
}

static constexpr auto DoesNotMatter = std::nullopt;

class PredictionComparator
{
public:
	PredictionComparator(const std::pair<unsigned char, ConfidenceLevel> prediction)
	: prediction_{prediction.first}
	, confidence_level_{prediction.second}
	{
		// DO NOTHING
	}

	void Is(const std::optional<unsigned char> other_prediction, const std::optional<ConfidenceLevel> other_confidence_level)
	{
		if (other_prediction)
		{
			EXPECT_EQ(prediction_, other_prediction);
		}

		if (other_confidence_level)
		{
			EXPECT_EQ(confidence_level_, other_confidence_level);
		}
	}
private:
	const unsigned char prediction_;
	const ConfidenceLevel confidence_level_;
};

PredictionComparator ExpectNextPredictionOf(INonCompressionAlgorithm& algorithm)
{
	return PredictionComparator{algorithm.GiveNextPrediction()};
}

} // namespace

class ExponentialSmoothingTest : public ::testing::Test
{
protected:
	ExponentialSmoothingTest();

	const unsigned char data_[7] = {1, 2, 3, 2, 1, 2, 3};
	const size_t size_ = ARRAY_SIZE(data_);

	PythonCompressor algorithm_;
};

ExponentialSmoothingTest::ExponentialSmoothingTest()
	: algorithm_{"exponential_smoothing"}
{
	algorithm_.RegisterFullTimeSeries(data_, size_);
}

TEST_F(ExponentialSmoothingTest, FirstPredictionIsNotConfident)
{
	ExpectNextPredictionOf(algorithm_).Is(DoesNotMatter, ConfidenceLevel::kNotConfident);
}

TEST_F(ExponentialSmoothingTest, SecondPredictionIsNotConfident)
{
	SkipPredictions(algorithm_, 1);
	ExpectNextPredictionOf(algorithm_).Is(DoesNotMatter, ConfidenceLevel::kNotConfident);
}

TEST_F(ExponentialSmoothingTest, ThirdPredictionIsConfidentAndCorrect)
{
	SkipPredictions(algorithm_, 2);
	ExpectNextPredictionOf(algorithm_).Is(2, ConfidenceLevel::kConfident);
}

TEST_F(ExponentialSmoothingTest, FourthPredictionIsConfidentAndCorrect)
{
	SkipPredictions(algorithm_, 3);
	ExpectNextPredictionOf(algorithm_).Is(3, ConfidenceLevel::kConfident);
}

TEST_F(ExponentialSmoothingTest, FifthPredictionIsConfidentAndCorrect)
{
	SkipPredictions(algorithm_, 4);
	ExpectNextPredictionOf(algorithm_).Is(2, ConfidenceLevel::kConfident);
}
