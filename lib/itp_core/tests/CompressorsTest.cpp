#include <gtest/gtest.h>

#include "../src/Compressors.h"
#include "CompressorMock.h"

#include <array>
#include <memory>

using namespace itp;
using namespace testing;

TEST(CompressorsPoolTest, InstanceCorrectlyCompressesSeveralTimes)
{
	unsigned char ts[]{0, 1, 1, 0, 1, 3, 0, 0, 0};
	auto compressors = MakeStandardCompressorsPool();
	compressors->SetAlphabetDescription({0, 3});

	size_t expected_size = BytesToBits(18);
	size_t obtained_size = compressors->Compress("zstd", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);

	expected_size = BytesToBits(17);
	obtained_size = compressors->Compress("zlib", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);

	expected_size = BytesToBits(18);
	obtained_size = compressors->Compress("zstd", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);

	expected_size = BytesToBits(15);
	obtained_size = compressors->Compress("ppmd", ts, sizeof(ts));
	EXPECT_EQ(obtained_size, expected_size);
}

TEST(CompressorsPoolTest, PassesSpecifiedAlphabetToInternalCompressor)
{
	auto compressor_mock = std::make_unique<CompressorMock>();
	EXPECT_CALL(*compressor_mock, SetTsParams(10, 20));

	auto pool = std::make_unique<CompressorsPool>();
	pool->RegisterCompressor("test", std::move(compressor_mock));
	pool->SetAlphabetDescription({10, 20});
}
