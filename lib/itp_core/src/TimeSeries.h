/**
 * @file   tseries.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Wed Apr 18 19:51:56 2018
 *
 * @brief  Contains implementation of words.
 */

#ifndef ITP_TSERIES_H_INCLUDED_
#define ITP_TSERIES_H_INCLUDED_

#include "PreprocInfo.h"
#include "PrimitiveDataTypes.h"

#include <initializer_list>
#include <iostream>
#include <utility>
#include <vector>

namespace itp
{

template<typename OrigType, typename NewType>
class PreprocessedTimeSeries : public PreprocInfo<OrigType>
{
public:
	using value_type = NewType;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator = typename PlainTimeSeries<NewType>::iterator;
	using const_iterator = typename PlainTimeSeries<NewType>::const_iterator;
	using difference_type = ptrdiff_t;
	using size_type = size_t;

	PreprocessedTimeSeries() = default;
	PreprocessedTimeSeries(size_t, NewType);
	explicit PreprocessedTimeSeries(const PlainTimeSeries<NewType>&);
	PreprocessedTimeSeries(std::initializer_list<NewType>);

	template<typename Iter>
	PreprocessedTimeSeries(Iter, Iter);

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

	void push_back(const NewType&);
	void push_back(NewType&&);

	const PlainTimeSeries<NewType>& to_plain_tseries() const;

private:
	PlainTimeSeries<NewType> series_;
};

template<typename OrigType, typename NewType>
std::ostream& operator<<(std::ostream&, const PreprocessedTimeSeries<OrigType, NewType>&);

} // namespace itp

template<typename OrigType, typename NewType>
itp::PreprocessedTimeSeries<OrigType, NewType>::PreprocessedTimeSeries(size_t sz, NewType init_elem)
	: series_(sz, init_elem)
{
	// DO NOTHING
}

template<typename OrigType, typename NewType>
itp::PreprocessedTimeSeries<OrigType, NewType>::PreprocessedTimeSeries(const PlainTimeSeries<NewType>& ts)
	: series_(std::begin(ts), std::end(ts))
{
	// DO NOTHING
}

template<typename OrigType, typename NewType>
itp::PreprocessedTimeSeries<OrigType, NewType>::PreprocessedTimeSeries(std::initializer_list<NewType> list)
	: series_(std::begin(list), std::end(list))
{
	// DO NOTHING
}

template<typename OrigType, typename NewType>
template<typename Iter>
itp::PreprocessedTimeSeries<OrigType, NewType>::PreprocessedTimeSeries(Iter first, Iter last)
	: series_(first, last)
{
	// DO NOTHING
}

template<typename OrigType, typename NewType>
void itp::PreprocessedTimeSeries<OrigType, NewType>::erase(iterator first, iterator last)
{
	series_.erase(first, last);
}

template<typename OrigType, typename NewType>
size_t itp::PreprocessedTimeSeries<OrigType, NewType>::size() const
{
	return series_.size();
}

template<typename OrigType, typename NewType>
bool itp::PreprocessedTimeSeries<OrigType, NewType>::empty() const
{
	return series_.size() == 0;
}

template<typename OrigType, typename NewType>
typename itp::PreprocessedTimeSeries<OrigType, NewType>::iterator
itp::PreprocessedTimeSeries<OrigType, NewType>::begin()
{
	return series_.begin();
}

template<typename OrigType, typename NewType>
typename itp::PreprocessedTimeSeries<OrigType, NewType>::const_iterator
itp::PreprocessedTimeSeries<OrigType, NewType>::begin() const
{
	return series_.begin();
}

template<typename OrigType, typename NewType>
typename itp::PreprocessedTimeSeries<OrigType, NewType>::iterator itp::PreprocessedTimeSeries<OrigType, NewType>::end()
{
	return series_.end();
}

template<typename OrigType, typename NewType>
typename itp::PreprocessedTimeSeries<OrigType, NewType>::const_iterator
itp::PreprocessedTimeSeries<OrigType, NewType>::end() const
{
	return series_.end();
}


template<typename OrigType, typename NewType>
typename itp::PreprocessedTimeSeries<OrigType, NewType>::const_iterator
itp::PreprocessedTimeSeries<OrigType, NewType>::cbegin() const
{
	return series_.cbegin();
}

template<typename OrigType, typename NewType>
typename itp::PreprocessedTimeSeries<OrigType, NewType>::const_iterator
itp::PreprocessedTimeSeries<OrigType, NewType>::cend() const
{
	return series_.cend();
}

template<typename OrigType, typename NewType>
NewType& itp::PreprocessedTimeSeries<OrigType, NewType>::operator[](size_t n)
{
	return series_[n];
}

template<typename OrigType, typename NewType>
const NewType& itp::PreprocessedTimeSeries<OrigType, NewType>::operator[](size_t n) const
{
	return series_[n];
}

template<typename OrigType, typename NewType>
void itp::PreprocessedTimeSeries<OrigType, NewType>::push_back(const NewType& to_insert)
{
	series_.push_back(to_insert);
}

template<typename OrigType, typename NewType>
void itp::PreprocessedTimeSeries<OrigType, NewType>::push_back(NewType&& to_insert)
{
	series_.push_back(std::forward<NewType>(to_insert));
}

template<typename OrigType, typename NewType>
const itp::PlainTimeSeries<NewType>& itp::PreprocessedTimeSeries<OrigType, NewType>::to_plain_tseries() const
{
	return series_;
}

template<typename OrigType, typename NewType>
std::ostream& itp::operator<<(std::ostream& ost, const PreprocessedTimeSeries<OrigType, NewType>& w)
{
	for (size_t i = 0; i < w.size(); ++i)
	{
		ost << w[i];
	}

	return ost;
}

#endif // ITP_TSERIES_H_INCLUDED_
