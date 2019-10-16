#ifndef CONTINUATION_H
#define CONTINUATION_H

#include "primitive_dtypes.h"

#include <vector>
#include <string>
#include <algorithm>

namespace itp {

static const unsigned int bits_in_byte = 8;

template<typename T>
struct Forecast_point {
  T point;

  // Confidence interval.
  T left_border;
  T right_border;
};

bool increment(std::vector<Symbol> &sequence, size_t min, size_t max);

template<typename T>
class Continuation {
 public:
  explicit Continuation(T init_symbol = 0);
  Continuation(size_t alphabet, size_t size, T init_symbol = 0);
  Continuation(std::initializer_list<T> list);

  const T &operator[](size_t ind) const;

  Continuation<T> operator++(int);
  Continuation<T> &operator++();

  size_t size() const;

  bool is_init() const;
  bool overflow() const;

  size_t get_alphabet_size() const;

  bool operator < (const Continuation<T> &rhs) const;
  bool operator > (const Continuation<T> &rhs) const;
  bool operator == (const Continuation<T> &rhs) const;
  bool operator != (const Continuation<T> &rhs) const;
  Continuation<T> operator / (T divisor) const;

  void push_back(const T &item);

  typename std::vector<T>::const_iterator cbegin() const;
  typename std::vector<T>::const_iterator cend() const;

 private:
  std::vector<T> continuation;
  size_t alphabet_size;
  bool is_overflow;
};

template<typename T>
class Continuations_generator {
 public:
  Continuations_generator(size_t alphabet, size_t size)
      : c(alphabet, size) {}

  Continuation<T> operator()() {
    return c++;
  }

 private:
  Continuation<T> c;
};

std::ostream & operator << (std::ostream &, const Continuation<Symbol> &);
bool operator < (const std::pair<Continuation<Symbol>, Double> &,
                 const std::pair<Continuation<Symbol>, Double> &);
} // of itp

template <typename T>
itp::Continuation<T>::Continuation(T init_symbol)
    : Continuation(init_symbol + 1, 1, init_symbol)
{
  // DO NOTHING
}

template <typename T>
itp::Continuation<T>::Continuation(size_t alphabet, size_t size, T init_symbol)
    : continuation(size, init_symbol), is_overflow(false)
{
  alphabet_size = alphabet;
  if (alphabet_size <= init_symbol) {
    throw std::range_error("In Continuation's constructor: init symbol out of the alphabet: "_s +
                           std::to_string(init_symbol) + " and " + std::to_string(alphabet_size) + ".");
  }
}

template <typename T>
itp::Continuation<T>::Continuation(std::initializer_list<T> list)
{
  continuation.resize(list.size());
  std::copy(begin(list), end(list), begin(continuation));
  alphabet_size = *std::max_element(begin(list), end(list)) + 1;
  is_overflow = false;
}

template <typename T>
const T& itp::Continuation<T>::operator[](size_t ind) const
{
  if (continuation.size() <= ind) {
    throw std::range_error("Index out of range");
  }

  return continuation[ind];
}

template <typename T>
itp::Continuation<T> itp::Continuation<T>::operator++(int)
{
  if (is_overflow) {
    return *this;
  }

  Continuation<T> prev(*this);
  is_overflow = !increment(continuation, 0, alphabet_size);
  return prev;
}

template <typename T>
itp::Continuation<T>& itp::Continuation<T>::operator++()
{
  if (!is_overflow) {
    is_overflow = !increment(continuation, 0, alphabet_size);
  }

  return *this;
}

template <typename T>
size_t itp::Continuation<T>::size() const
{
  return continuation.size();
}

template <typename T>
bool itp::Continuation<T>::is_init() const
{
  return std::all_of(begin(continuation), end(continuation), [](T item) { return item == 0; });
}

template <typename T>
bool itp::Continuation<T>::overflow() const
{
  return is_overflow;
}

template <typename T>
size_t itp::Continuation<T>::get_alphabet_size() const
{
  return alphabet_size;
}

template <typename T>
bool itp::Continuation<T>::operator < (const Continuation<T> &rhs) const
{
  if (continuation.size() != rhs.size()) {
    throw std::invalid_argument("In operator <: continuations must have the same length.");
  }

  for (size_t i = continuation.size(); i > 0; --i) {
    if (continuation[i-1] < rhs[i-1]) {
      return true;
    } else if (continuation[i-1] > rhs[i-1]) {
      return false;
    }
  }

  return false;
}

template <typename T>
bool itp::Continuation<T>::operator > (const Continuation<T> &rhs) const
{
  if (continuation.size() != rhs.size()) {
    throw std::invalid_argument("In operator >: Continuations must have the same length.");
  }

  for (size_t i = continuation.size(); i > 0; --i) {
    if (continuation[i-1] > rhs[i-1]) {
      return true;
    } else if (continuation[i-1] < rhs[i-1]) {
      return false;
    }
  }

  return false;
}

template <typename T>
bool itp::Continuation<T>::operator == (const Continuation<T> &rhs) const
{
  if (continuation.size() != rhs.size()) {
    return false;
  }
  
  for (size_t i = continuation.size(); i > 0; --i) {
    if (continuation[i-1] != rhs[i-1]) {
      return false;
    }
  }

  return true;
}

template <typename T>
bool itp::Continuation<T>::operator != (const Continuation<T> &rhs) const
{
  return !(*this == rhs);
}

template <typename T>
itp::Continuation<T> itp::Continuation<T>::operator / (T divisor) const
{
  Continuation<T> result(*this);
  std::transform(begin(continuation), end(continuation), begin(result.continuation), [&divisor](T value) { return value / divisor; });
  result.alphabet_size /= divisor;
  result.is_overflow = false;

  return result;
}

template <typename T>
void itp::Continuation<T>::push_back(const T &item)
{
  if (item >= alphabet_size) {
    throw std::range_error("In Continuation::push_back: item '"_s + std::to_string(item) +
                           "' is greater than alphabet size " + std::to_string(alphabet_size) + ".");
  }

  continuation.push_back(item);
}

template <typename T>
typename std::vector<T>::const_iterator itp::Continuation<T>::cbegin() const
{
  return continuation.cbegin();
}

template <typename T>
typename std::vector<T>::const_iterator itp::Continuation<T>::cend() const
{
  return continuation.cend();
}

namespace std {
template<>
struct hash<itp::Continuation<itp::Symbol>> {
  size_t operator()(const itp::Continuation<itp::Symbol> &c) const {
    size_t h = 0;
    for (size_t i = 0; i < c.size(); ++i) {
      h = (2 * h + c[i]) % q;
    }

    return h;
  }

  static const size_t q = 32452843;
};

template<>
struct equal_to<itp::Continuation<itp::Symbol>> {
  bool operator()(const itp::Continuation<itp::Symbol> &r,
                  const itp::Continuation<itp::Symbol> &r2) const {
    return r == r2;
  }
};
} // of std

#endif // CONTINUATION_H
