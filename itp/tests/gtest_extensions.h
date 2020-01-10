#ifndef ITP_GTEST_EXTENSIONS_H_INCLUDED_
#define ITP_GTEST_EXTENSIONS_H_INCLUDED_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>
#include <type_traits>

namespace itp {

template <typename Container1, typename Container2>
void ExpectDoubleContainersEq(const Container1 &container1, const Container2 &container2) {
  ASSERT_EQ(container1.size(), container2.size());

  auto p = cbegin(container1);
  auto q = cbegin(container2);
  while (p != cend(container1)) {
    EXPECT_DOUBLE_EQ(*p++, *q++);
  }
}

template <typename Container1, typename Container2>
void ExpectContainersEq(const Container1 &container1, const Container2 &container2) {
  ASSERT_EQ(container1.size(), container2.size());

  auto p = cbegin(container1);
  auto q = cbegin(container2);
  while (p != cend(container1)) {
    EXPECT_EQ(*p++, *q++);
  }
}

template <typename Container>
void ExpectContainerEmpty(const Container &container) {
  EXPECT_EQ(container.size(), 0);
}

template <typename ContainerRange1, typename ContainerRange2>
void ExpectContainerRangesEq(const ContainerRange1 &range1, const ContainerRange2 &range2)
{
	ASSERT_EQ(range1.size(), range2.size());

	auto p = std::cbegin(range1);
	auto q = std::cbegin(range2);
	while (p != std::cend(range1)) {
		ExpectContainersEq(*p++, *q++);
	}
}
} // itp

#endif // ITP_GTEST_EXTENSIONS_H_INCLUDED_
