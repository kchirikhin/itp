#ifndef ITP_GTEST_EXTENSIONS_H_INCLUDED_
#define ITP_GTEST_EXTENSIONS_H_INCLUDED_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>
#include <type_traits>

namespace itp {

template <typename Container1, typename Container2>
void ExpectDoubleContainersEq(const Container1 &container1, const Container2 &container2) {
  static_assert(std::is_floating_point<typename Container1::value_type>::value,
                "Container1 must contain floating-point numbers");
  static_assert(std::is_floating_point<typename Container2::value_type>::value,
                "Container2 must contain floating-point numbers"); 
  ASSERT_EQ(container1.size(), container2.size());

  auto p = container1.begin();
  auto q = container2.begin();
  while (p != container1.end()) {
    EXPECT_DOUBLE_EQ(*p++, *q++);
  }
}
} // itp

#endif // ITP_GTEST_EXTENSIONS_H_INCLUDED_
