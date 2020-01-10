/**
 * Functions for sampling and desampling real-valued time series.
 */

#ifndef ITP_SAMPLER_H_INCLUDED_
#define ITP_SAMPLER_H_INCLUDED_

#include "dtypes.h"

#include <cassert>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace itp
{

/**
 * Converts real-valued series to a discrete one, or converts discrete series to a zero-aligned series (i.e. series of
 * numbers 0...n for some n).
 *
 * @tparam OriginalType Type of values of the series to convert. Needed for inverse transformation (with loss of
 * accuracy in the real-valued case).
 */
template<typename OriginalType>
class Sampler;

template<>
class Sampler<Double>
{
public:
	Preprocessed_tseries<Double, Symbol> Transform(const Preprocessed_tseries<Double, Double> &, size_t);

	Double InverseTransform(Symbol, const Preproc_info<Double> &);

private:

	double indent_ = 0.1;
};

// TODO Make it possible to use negative numbers.
template<>
class Sampler<Symbol>
{
public:
	Preprocessed_tseries<Double, Symbol> Transform(const Preprocessed_tseries<Double, Symbol> &);

	Double InverseTransform(Symbol, const Preproc_info<Double> &);
};

template<>
class Sampler<VectorDouble>
{
public:
	Preprocessed_tseries<VectorDouble, Symbol> Transform(const Preprocessed_tseries<VectorDouble, VectorDouble> &,
														 size_t);

	VectorDouble InverseTransform(Symbol, const Preproc_info<VectorDouble> &);

private:
	double indent_ = 0.1;
};

template<>
class Sampler<VectorSymbol>
{
public:
	Preprocessed_tseries<VectorDouble, Symbol> Transform(const Preprocessed_tseries<VectorDouble, VectorSymbol> &);

	VectorDouble InverseTransform(Symbol, const Preproc_info<VectorDouble> &);
};

template<typename T>
using SamplerPtr = std::shared_ptr<Sampler<T>>;

template<typename ForwardIterator, typename Value>
typename std::iterator_traits<ForwardIterator>::value_type pointwise_operation(
		ForwardIterator first, ForwardIterator last, std::function<Value(Value, Value)> op)
{
	using ElemType = typename std::iterator_traits<ForwardIterator>::value_type;

	ElemType to_return = *first;
	while (++first != last)
	{
		auto to_return_iter = std::begin(to_return);
		for (auto value : *first)
		{
			if (op(value, *to_return_iter))
			{
				*to_return_iter = value;
			}
			++to_return_iter;
		}
	}

	return to_return;
}

template<typename ForwardIterator>
typename std::iterator_traits<ForwardIterator>::value_type pointwise_max_elements(ForwardIterator first,
																				  ForwardIterator last)
{
	return pointwise_operation<ForwardIterator, Double>(first, last, std::greater<Double>());
}

template<typename ForwardIterator>
typename std::iterator_traits<ForwardIterator>::value_type pointwise_min_elements(ForwardIterator first,
																				  ForwardIterator last)
{
	return pointwise_operation<ForwardIterator, Double>(first, last, std::less<Double>());
}

Symbol ConvertNumberToDec(const VectorSymbol &, size_t);

VectorSymbol ConvertDecToNumber(Symbol, size_t);

} // of namespace itp

#endif // ITP_SAMPLER_H_INCLUDED_
