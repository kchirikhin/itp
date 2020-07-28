//
// Created by kon on 11.01.2020.
//

#include <gtest/gtest.h>

#include <compressors.h>
#include "compressor_mock.h"

#include <array>
#include <memory>

using namespace itp;
using namespace testing;

TEST(CompressorsPoolTest, InstanceCorrectlyCompressesSeveralTimes)
{
	unsigned char ts[]{0, 1, 1, 0, 1, 3, 0, 0, 0};
	auto compressors = MakeStandardCompressorsPool({0, 3});

	size_t expected_size = 18;
	size_t obtained_size = compressors->Compress("zstd", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);

	expected_size = 17;
	obtained_size = compressors->Compress("zlib", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);

	expected_size = 18;
	obtained_size = compressors->Compress("zstd", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);

	expected_size = 15;
	obtained_size = compressors->Compress("ppmd", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);
}

TEST(CompressorsPoolTest, PassesSpecifiedAlphabetToInternalCompressor)
{
	auto compressor_mock = std::make_unique<CompressorMock>();
	EXPECT_CALL(*compressor_mock, SetTsParams(10, 20));

	auto pool = std::make_unique<CompressorsPool>(AlphabetDescription{10, 20});
	pool->RegisterCompressor("test", std::move(compressor_mock));
}

TEST(CompressorsPoolTest, ResetsAlphabetDescriptionForAllRegisteredCompressors)
{
	constexpr size_t compressors_number = 3;
	std::array<std::unique_ptr<CompressorMock>, compressors_number> compressors;

	auto pool = std::make_unique<CompressorsPool>(AlphabetDescription{10, 20});
	for (size_t i = 0; i < compressors_number; ++i)
	{
		auto compressor = std::make_unique<CompressorMock>();
		EXPECT_CALL(*compressor, SetTsParams(10, 20));
		EXPECT_CALL(*compressor, SetTsParams(30, 40));

		pool->RegisterCompressor(std::to_string(i), std::move(compressor));
	}

	pool->ResetAlphabetDescription(AlphabetDescription{30, 40});
}

TEST(ArimaTest, ReturnsZeroOnEmptyInput)
{
	const unsigned char* data = nullptr;
	PythonCompressor arima{"arima.py"};
	EXPECT_EQ(arima(data, 0, nullptr), 0);
}

TEST(ArimaTest, CorrectlyWorksOnOneLetter)
{
	const unsigned char data[] = {5};
	PythonCompressor arima{"arima.py"};
	EXPECT_EQ(arima(data, sizeof(data) / sizeof(unsigned char), nullptr), static_cast<size_t>(ceil(-log2(1.0/256))));
}

TEST(ArimaTest, CorrectlyWorksOnTwoLetters)
{
	const unsigned char data[] = {5, 7};
	PythonCompressor arima{"arima.py"};
	EXPECT_EQ(arima(data, sizeof(data) / sizeof(unsigned char), nullptr), static_cast<size_t>(ceil(-log2(1.0/(256 * 256)))));
}