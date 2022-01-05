/**
 * @file   tseries.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Wed Apr 18 19:51:56 2018
 *
 * @brief  Contains implementation of words.
 *
 *
 */

#ifndef ITP_TSERIES_H_INCLUDED_
#define ITP_TSERIES_H_INCLUDED_

#include "PrimitiveDataTypes.h"
#include "PreprocInfo.h"

#include <initializer_list>
#include <iostream>
#include <utility>
#include <vector>

namespace itp {

template <typename Orig_type, typename New_type>
class Preprocessed_tseries : public Preproc_info<Orig_type> {
 public:
  using value_type = New_type;
  using reference = value_type &;
  using const_reference = const value_type &;
  using iterator = typename PlainTimeSeries<New_type>::iterator;
  using const_iterator = typename PlainTimeSeries<New_type>::const_iterator;
  using difference_type = ptrdiff_t;
  using size_type = size_t;

  Preprocessed_tseries() = default;
  Preprocessed_tseries(size_t, New_type);
  explicit Preprocessed_tseries(const PlainTimeSeries<New_type> &);
  Preprocessed_tseries(std::initializer_list<New_type>);

  template <typename Iter>
  Preprocessed_tseries(Iter, Iter);
        
  size_t size() const;
  bool empty() const;
  void erase(iterator, iterator);

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;
        
  const_iterator cbegin() const;
  const_iterator cend() const;

  reference operator[](size_t);
  const_reference operator[](size_t) const;

  void push_back(const New_type &);
  void push_back(New_type &&);

  const PlainTimeSeries<New_type>& to_plain_tseries() const;

 private:
  PlainTimeSeries<New_type> series;
};

template <typename Orig_type, typename New_type>
std::ostream & operator << (std::ostream &, const Preprocessed_tseries<Orig_type, New_type> &);
} // of itp

template <typename Orig_type, typename New_type>
itp::Preprocessed_tseries<Orig_type, New_type>::Preprocessed_tseries(size_t sz, New_type init_elem)
    : series(sz, init_elem) {
  // DO NOTHING
}

template <typename Orig_type, typename New_type>
itp::Preprocessed_tseries<Orig_type, New_type>::Preprocessed_tseries(const PlainTimeSeries<New_type> &ts)
    : series(std::begin(ts), std::end(ts)) {
  // DO NOTHING
}

template <typename Orig_type, typename New_type>
itp::Preprocessed_tseries<Orig_type, New_type>::Preprocessed_tseries(std::initializer_list<New_type> list)
    : series(std::begin(list), std::end(list)) {
  // DO NOTHING
}

template <typename Orig_type, typename New_type>
template <typename Iter>
itp::Preprocessed_tseries<Orig_type, New_type>::Preprocessed_tseries(Iter first, Iter last)
    : series(first, last) {
  // DO NOTHING
}

template <typename Orig_type, typename New_type>
void itp::Preprocessed_tseries<Orig_type, New_type>::erase(iterator first, iterator last) {
  series.erase(first, last);
}

template <typename Orig_type, typename New_type>
size_t itp::Preprocessed_tseries<Orig_type, New_type>::size() const {
  return series.size();
}

template <typename Orig_type, typename New_type>
bool itp::Preprocessed_tseries<Orig_type, New_type>::empty() const {
  return series.size() == 0;
}

template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::iterator
itp::Preprocessed_tseries<Orig_type, New_type>::begin() {
  return series.begin();
}

template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::const_iterator
itp::Preprocessed_tseries<Orig_type, New_type>::begin() const {
  return series.begin();
}

template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::iterator
itp::Preprocessed_tseries<Orig_type, New_type>::end() {
  return series.end();
}

template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::const_iterator
itp::Preprocessed_tseries<Orig_type, New_type>::end() const {
  return series.end();
}


template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::const_iterator itp::Preprocessed_tseries<Orig_type, New_type>::cbegin() const {
  return series.cbegin();
}

template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::const_iterator itp::Preprocessed_tseries<Orig_type, New_type>::cend() const {
  return series.cend();
}

template <typename Orig_type, typename New_type>
New_type& itp::Preprocessed_tseries<Orig_type, New_type>::operator[](size_t n) {
  return series[n];
}

template <typename Orig_type, typename New_type>
const New_type& itp::Preprocessed_tseries<Orig_type, New_type>::operator[](size_t n) const {
  return series[n];
}

template <typename Orig_type, typename New_type>
void itp::Preprocessed_tseries<Orig_type, New_type>::push_back(const New_type &to_insert) {
  series.push_back(to_insert);
}

template <typename Orig_type, typename New_type>
void itp::Preprocessed_tseries<Orig_type, New_type>::push_back(New_type &&to_insert) {
  series.push_back(std::forward<New_type>(to_insert));
}

template <typename Orig_type, typename New_type>
const itp::PlainTimeSeries<New_type> & itp::Preprocessed_tseries<Orig_type, New_type>::to_plain_tseries() const {
  return series;
}

template <typename Orig_type, typename New_type>
std::ostream & itp::operator << (std::ostream &ost, const Preprocessed_tseries<Orig_type, New_type> &w) {
  for (size_t i = 0; i < w.size(); ++i) {
    ost << w[i];
  }

  return ost;
}

#endif // ITP_TSERIES_H_INCLUDED_
