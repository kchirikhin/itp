//
// Created by kon on 11.01.2020.
//

#include <gtest/gtest.h>

#include <compressors.h>

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
