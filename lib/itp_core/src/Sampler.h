/**
 * Functions for sampling and desampling real-valued time series.
 */

#ifndef ITP_SAMPLER_H_INCLUDED_
#define ITP_SAMPLER_H_INCLUDED_

#include "Types.h"

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
 * \tparam OriginalType Type of values of the series to convert. Needed for inverse transformation (with loss of
 * accuracy in the real-valued case).
 */
template<typename OriginalType>
class Sampler;

template<>
class Sampler<Double>
{
public:
	PreprocessedTimeSeries<Double, Symbol> Transform(const PreprocessedTimeSeries<Double, Double>&, size_t);

	Double InverseTransform(Symbol, const PreprocInfo<Double>&);

private:
	double indent_ = 0.1;
};

// TODO Make it possible to use negative numbers.
template<>
class Sampler<Symbol>
{
public:
	PreprocessedTimeSeries<Double, Symbol> Transform(const PreprocessedTimeSeries<Double, Symbol>&);

	Double InverseTransform(Symbol, const PreprocInfo<Double>&);
};

template<>
class Sampler<VectorDouble>
{
public:
	PreprocessedTimeSeries<VectorDouble, Symbol> Transform(
		const PreprocessedTimeSeries<VectorDouble, VectorDouble>&,
		size_t);

	VectorDouble InverseTransform(Symbol, const PreprocInfo<VectorDouble>&);

private:
	double indent_ = 0.1;
};

template<>
class Sampler<VectorSymbol>
{
public:
	PreprocessedTimeSeries<VectorDouble, Symbol> Transform(const PreprocessedTimeSeries<VectorDouble, VectorSymbol>&);

	VectorDouble InverseTransform(Symbol, const PreprocInfo<VectorDouble>&);
};

template<typename T>
using SamplerPtr = std::shared_ptr<Sampler<T>>;

template<typename ForwardIterator, typename Value>
typename std::iterator_traits<ForwardIterator>::value_type PointwiseOperation(
	ForwardIterator first,
	ForwardIterator last,
	std::function<Value(Value, Value)> op)
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
typename std::iterator_traits<ForwardIterator>::value_type PointwiseMaxElements(
	ForwardIterator first,
	ForwardIterator last)
{
	return PointwiseOperation<ForwardIterator, Double>(first, last, std::greater<Double>());
}

template<typename ForwardIterator>
typename std::iterator_traits<ForwardIterator>::value_type PointwiseMinElements(
	ForwardIterator first,
	ForwardIterator last)
{
	return PointwiseOperation<ForwardIterator, Double>(first, last, std::less<Double>());
}

template<typename OriginalType>
auto InitPreprocessedTs(const PlainTimeSeries<OriginalType>&);

inline auto InitPreprocessedTs(const PlainTimeSeries<Double>& points)
{
	return PreprocessedTimeSeries<Double, Double>{points};
}

inline auto InitPreprocessedTs(const PlainTimeSeries<Symbol>& points)
{
	return PreprocessedTimeSeries<Double, Symbol>{points};
}

inline auto InitPreprocessedTs(const PlainTimeSeries<VectorDouble>& points)
{
	return PreprocessedTimeSeries<VectorDouble, VectorDouble>{points};
}

inline auto InitPreprocessedTs(const PlainTimeSeries<VectorSymbol>& points)
{
	return PreprocessedTimeSeries<VectorDouble, VectorSymbol>{points};
}

Symbol ConvertNumberToDec(const VectorSymbol&, size_t);

VectorSymbol ConvertDecToNumber(Symbol, size_t);

} // namespace itp

#endif // ITP_SAMPLER_H_INCLUDED_
