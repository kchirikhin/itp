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

  auto p = begin(container1);
  auto q = begin(container2);
  while (p != end(container1)) {
    EXPECT_DOUBLE_EQ(*p++, *q++);
  }
}

template <typename Container1, typename Container2>
void ExpectContainersEq(const Container1 &container1, const Container2 &container2) {
  ASSERT_EQ(container1.size(), container2.size());

  auto p = begin(container1);
  auto q = begin(container2);
  while (p != end(container1)) {
    EXPECT_EQ(*p++, *q++);
  }
}

template <typename Container>
void ExpectContainerEmpty(const Container &container) {
  EXPECT_EQ(container.size(), 0);
}
} // itp

#endif // ITP_GTEST_EXTENSIONS_H_INCLUDED_
