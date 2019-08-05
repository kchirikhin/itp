#include <tseries.h>

#include <gtest/gtest.h>

using namespace itp;

TEST(PlainTseriesTest, CreatingEmptySeries) {
    Plain_tseries<int> series;

    EXPECT_EQ(series.size(), 0);
}

TEST(PlainTseriesTest, CreatingSeriesOfSizeOneWithUnspecifiedDefault) {
    Plain_tseries<int> series(1);

    ASSERT_EQ(series.size(), 1);
    EXPECT_EQ(series[0], int());
}

TEST(PlainTseriesTest, CreatingSeriesOfSizeOneWithSpecifiedDefault) {
    Plain_tseries<int> series(1, 2);

    ASSERT_EQ(series.size(), 1);
    EXPECT_EQ(series[0], 2);
}

TEST(PlainTseriesTest, CreatingSeriesFromInitializerList) {
    Plain_tseries<int> series {1, 2, 3};

    ASSERT_EQ(series.size(), 3);
    for (size_t i = 0; i < series.size(); ++i) {
        EXPECT_EQ(series[i], i+1);
    }
}

TEST(PlainTseriesTest, PushingElementToEmptySeries) {
    Plain_tseries<float> series;
    series.push_back(.5);

    EXPECT_EQ(series.size(), 1);
}

TEST(PlainTseriesTest, ClearSeriesWithOneElement) {
    Plain_tseries<float> series;
    series.push_back(.5);
    series.clear();

    EXPECT_EQ(series.size(), 0);
}

TEST(PlainTseriesTest, BeginIteratorEqualsToEndIteratorOnEmptySeries) {
    Plain_tseries<float> series;
    
    EXPECT_EQ(series.begin(), series.end());
}

TEST(PlainTseriesTest, CbeginIteratorEqualsToCendIteratorOnEmptySeries) {
    Plain_tseries<float> series;
    
    EXPECT_EQ(series.cbegin(), series.cend());
}

TEST(PlainTseriesTest, IndexingWorksOnSeriesWithOneElement) {
    Plain_tseries<char> series;
    series.push_back('a');

    EXPECT_EQ(series[0], 'a');
}
