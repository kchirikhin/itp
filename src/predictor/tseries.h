/**
 * @file   tseries.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Wed Apr 18 19:51:56 2018
 *
 * @brief  Contains implementation of words.
 *
 *
 */

#ifndef TSERIES_H_INCLUDED
#define TSERIES_H_INCLUDED

#include "primitive_dtypes.h"
#include "preproc_info.h"

#include <vector>
#include <iostream>
#include <initializer_list>

namespace itp {
    template <typename Orig_type, typename New_type>
    class Preprocessed_tseries : public Preproc_info<Orig_type> {
    public:
        using iterator = typename Plain_tseries<New_type>::iterator;
        using const_iterator = typename Plain_tseries<New_type>::const_iterator;

        Preprocessed_tseries() = default;
        Preprocessed_tseries(size_t, New_type);
        Preprocessed_tseries(const Plain_tseries<New_type> &);

        template <typename Iter>
        Preprocessed_tseries(Iter, Iter);
        
        size_t size() const;
        void erase(iterator, iterator);

        iterator begin();
        iterator end();
        
        const_iterator cbegin() const;
        const_iterator cend() const;

        New_type& operator[](size_t);
        const New_type& operator[](size_t) const;

        const Plain_tseries<New_type> & to_plain_tseries() const;

    private:
        Plain_tseries<New_type> series;
    };

    template <typename Orig_type, typename New_type>
    std::ostream & operator << (std::ostream &, const Preprocessed_tseries<Orig_type, New_type> &);
} // of itp

template <typename Orig_type, typename New_type>
itp::Preprocessed_tseries<Orig_type, New_type>::Preprocessed_tseries(size_t sz, New_type init_elem)
    : series(sz, init_elem) {}

template <typename Orig_type, typename New_type>
itp::Preprocessed_tseries<Orig_type, New_type>::Preprocessed_tseries(const Plain_tseries<New_type> &ts)
    : series(std::begin(ts), std::end(ts)) {}

template <typename Orig_type, typename New_type>
template <typename Iter>
itp::Preprocessed_tseries<Orig_type, New_type>::Preprocessed_tseries(Iter first, Iter last)
    : series(first, last) {}

template <typename Orig_type, typename New_type>
void itp::Preprocessed_tseries<Orig_type, New_type>::erase(iterator first, iterator last) {
    series.erase(first, last);
}

template <typename Orig_type, typename New_type>
size_t itp::Preprocessed_tseries<Orig_type, New_type>::size() const {
    return series.size();
}

template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::iterator
itp::Preprocessed_tseries<Orig_type, New_type>::begin() {
    return series.begin();
}

template <typename Orig_type, typename New_type>
typename itp::Preprocessed_tseries<Orig_type, New_type>::iterator
itp::Preprocessed_tseries<Orig_type, New_type>::end() {
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
const itp::Plain_tseries<New_type> & itp::Preprocessed_tseries<Orig_type, New_type>::to_plain_tseries() const {
    return series;
}

template <typename Orig_type, typename New_type>
std::ostream & itp::operator << (std::ostream &ost, const Preprocessed_tseries<Orig_type, New_type> &w) {
    for (size_t i = 0; i < w.size(); ++i) {
        ost << w[i];
    }

    return ost;
}

#endif // TSERIES_H_INCLUDED
