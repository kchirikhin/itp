#include "../src/TimeSeries.h"

#include <gtest/gtest.h>

using namespace itp;

TEST(PlainTseriesTest, CreatingEmptySeries) {
    PlainTimeSeries<int> series;

    EXPECT_EQ(series.size(), 0);
}

TEST(PlainTseriesTest, CreatingSeriesOfSizeOneWithUnspecifiedDefault) {
    PlainTimeSeries<int> series(1);

    ASSERT_EQ(series.size(), 1);
    EXPECT_EQ(series[0], int());
}

TEST(PlainTseriesTest, CreatingSeriesOfSizeOneWithSpecifiedDefault) {
    PlainTimeSeries<int> series(1, 2);

    ASSERT_EQ(series.size(), 1);
    EXPECT_EQ(series[0], 2);
}

TEST(PlainTseriesTest, CreatingSeriesFromInitializerList) {
    PlainTimeSeries<int> series {1, 2, 3};

    ASSERT_EQ(series.size(), 3);
    for (size_t i = 0; i < series.size(); ++i) {
        EXPECT_EQ(series[i], i+1);
    }
}

TEST(PlainTseriesTest, PushingElementToEmptySeries) {
    PlainTimeSeries<float> series;
    series.push_back(.5);

    EXPECT_EQ(series.size(), 1);
}

TEST(PlainTseriesTest, ClearSeriesWithOneElement) {
    PlainTimeSeries<float> series;
    series.push_back(.5);
    series.clear();

    EXPECT_EQ(series.size(), 0);
}

TEST(PlainTseriesTest, BeginIteratorEqualsToEndIteratorOnEmptySeries) {
    PlainTimeSeries<float> series;
    
    EXPECT_EQ(series.begin(), series.end());
}

TEST(PlainTseriesTest, CbeginIteratorEqualsToCendIteratorOnEmptySeries) {
    PlainTimeSeries<float> series;
    
    EXPECT_EQ(series.cbegin(), series.cend());
}

TEST(PlainTseriesTest, IndexingWorksOnSeriesWithOneElement) {
    PlainTimeSeries<char> series;
    series.push_back('a');

    EXPECT_EQ(series[0], 'a');
}
