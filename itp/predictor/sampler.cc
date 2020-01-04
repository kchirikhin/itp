#include "sampler.h"

#include <algorithm>
#include <cmath>
#include "itp_exceptions.h"
#include <functional>

namespace itp
{

template<typename T>
Double GeneralizedInverseTransform(Symbol s, const Preproc_info<T> &info)
{
	if (!info.is_sampled())
	{
		return s;
	}

	assert(!info.get_desample_table().empty());
	assert(s < info.get_desample_table().size());

	return info.get_desample_table()[s];
}

Preprocessed_tseries<Double, Symbol>
Sampler<Double>::Transform(const Preprocessed_tseries<Double, Double> &points, size_t N)
{
	if (points.size() == 1)
	{
		throw SeriesTooShortError("Time series to transform must contain at least 2 elems or be empty");
	}

	if (points.empty())
	{
		return {};
	}

	auto min = *std::min_element(points.cbegin(), points.cend());
	auto max = *std::max_element(points.cbegin(), points.cend());
	const auto width = fabs(max - min);

	min -= (width * indent_);
	max += (width * indent_);
	const auto delta = (max - min) / N;

	PlainTimeSeries<Symbol> sampled_ts(points.size());
	for (size_t i = 0; i < points.size(); ++i)
	{
		sampled_ts[i] = static_cast<PlainTimeSeries<Symbol>::value_type>(floor((points[i] - min) / delta));

		// This event occurs for the maximal element of the time series.
		if (sampled_ts[i] > N - 1)
		{
			sampled_ts[i] = static_cast<Symbol>(N - 1);
		}
	}

	Preprocessed_tseries<Double, Symbol> to_return(sampled_ts);
	to_return.copy_preprocessing_info_from(points);

	std::vector<Double> desample_table(N);
	for (size_t i = 0; i < N; ++i)
	{
		desample_table[i] = min + i * delta + delta / 2;
	}

	to_return.set_desample_table(desample_table);
	to_return.set_desample_indent(indent_);
	to_return.set_sampling_alphabet(N);

	return to_return;
}

Double Sampler<Double>::InverseTransform(Symbol s, const Preproc_info<Double> &info)
{
	return GeneralizedInverseTransform(s, info);
}

Preprocessed_tseries<Double, Symbol>
Sampler<Symbol>::Transform(const Preprocessed_tseries<Double, Symbol> &points)
{
	if (points.empty())
	{
		return {};
	}

	using namespace std::placeholders;

	const auto min_point = *std::min_element(points.cbegin(), points.cend());
	const auto max_point = *std::max_element(points.cbegin(), points.cend());
	PlainTimeSeries<Symbol> normalized_points(points.size());
	std::transform(points.cbegin(), points.cend(), begin(normalized_points),
				   std::bind(std::minus<>(), _1, min_point));
	assert(*std::min_element(begin(normalized_points), end(normalized_points)) == 0);

	std::vector<Double> desample_table(max_point - min_point + 1);
	for (size_t i = 0; i < desample_table.size(); ++i)
	{
		desample_table[i] = i + min_point;
	}

	Preprocessed_tseries<Double, Symbol> to_return(normalized_points);
	to_return.copy_preprocessing_info_from(points);
	to_return.set_desample_table(desample_table);
	to_return.set_sampling_alphabet(max_point - min_point + 1);

	return to_return;
}

Double Sampler<Symbol>::InverseTransform(Symbol s, const Preproc_info<Double> &info)
{
	return GeneralizedInverseTransform(s, info);
}

VectorSymbol ToVectorSymbol(const VectorDouble &to_convert)
{
	VectorSymbol to_return(to_convert.size());
	std::copy(std::cbegin(to_convert), std::cend(to_convert), std::begin(to_return));

	return to_return;
}

void EnforceMaxValue(VectorSymbol *vec, Symbol max_value)
{
	assert(vec != nullptr);

	using namespace std::placeholders;
	std::replace_if(std::begin(*vec), std::end(*vec), std::bind(std::greater<Symbol>(), _1, max_value), max_value);
}

Preprocessed_tseries<VectorDouble, Symbol>
Sampler<VectorDouble>::Transform(const Preprocessed_tseries<VectorDouble, VectorDouble> &points,
								 size_t N)
{
	if (points.size() == 1)
	{
		throw SeriesTooShortError("Time series to transform must contain at least 2 elems or be empty");
	}

	if (points.empty())
	{
		return {};
	}

	const auto kCountOfSeries = points[0].size();
	const size_t kNewAlphabetSize = pow(N, kCountOfSeries);

	const auto kNumbersInByte = 256u;
	if (kNumbersInByte <= kNewAlphabetSize)
	{
		throw IntervalsCountError("Symbols of the alphabet after transformation cannot be represented with 1 byte");
	}

	auto mins = pointwise_min_elements(points.cbegin(), points.cend());
	assert(mins.size() == points[0].size());

	auto maxs = pointwise_max_elements(points.cbegin(), points.cend());
	assert(maxs.size() == points[0].size());

	VectorDouble widths = abs(maxs - mins);
	mins -= (widths * indent_);
	maxs += (widths * indent_);
	VectorDouble deltas = (maxs - mins) / N;

	PlainTimeSeries<VectorSymbol> sampled_ts(points.size());
	for (size_t i = 0; i < points.size(); ++i)
	{
		sampled_ts[i] = ToVectorSymbol((points[i] - mins) / deltas);
		EnforceMaxValue(&sampled_ts[i], N - 1);
	}

	Preprocessed_tseries<VectorDouble, Symbol> to_return;
	for (auto &vec : sampled_ts)
	{
		to_return.push_back(ConvertNumberToDec(vec, N));
	}
	to_return.copy_preprocessing_info_from(points);

	std::vector<VectorDouble> desample_table(kCountOfSeries, VectorDouble(N));
	for (size_t i = 0; i < kCountOfSeries; ++i)
	{
		for (size_t j = 0; j < N; ++j)
		{
			desample_table[i][j] = mins[i] + j * deltas[i] + deltas[i] / 2;
		}
	}

	to_return.set_desample_table(desample_table);
	to_return.set_desample_indent(indent_);
	to_return.set_sampling_alphabet(kNewAlphabetSize);

	return to_return;
}

inline auto NumberOfDigits(const VectorSymbol &num)
{
	return num.size();
}

VectorDouble Sampler<VectorDouble>::InverseTransform(Symbol s, const Preproc_info<VectorDouble> &info)
{
	const auto &kConversionTable = info.get_desample_table();
	const auto kNumberOfSeries = kConversionTable.size();
	const auto kSingeSeriesAlphabet = static_cast<size_t>(pow(info.get_sampling_alphabet(),
															  1. / static_cast<ssize_t>(kNumberOfSeries)));
	auto decomposed_number = ConvertDecToNumber(s, kSingeSeriesAlphabet);

	if (NumberOfDigits(decomposed_number) > kNumberOfSeries)
	{
		throw RangeError("Inverse conversion error: passed number has more digits than the number of time series");
	}

	VectorDouble to_return(kNumberOfSeries);
	size_t i = 0;
	for (i = 0; i < NumberOfDigits(decomposed_number); ++i)
	{
		assert(decomposed_number[i] < kConversionTable[i].size());
		to_return[i] = kConversionTable[i][decomposed_number[i]];
	}

	while (i != kNumberOfSeries)
	{
		to_return[i] = kConversionTable[i][0];
		++i;
	}

	return to_return;
}

Preprocessed_tseries<VectorDouble, Symbol>
Sampler<VectorSymbol>::Transform(const Preprocessed_tseries<VectorDouble, VectorSymbol> &points)
{
	/*if (points.empty()) {
	  return {};
	  }*/

	/*using namespace std::placeholders;

	auto min_points = pointwise_min_elements(points.cbegin(), points.cend());
	assert(min_points.size() == points[0].size());

	auto max_points = pointwise_max_elements(points.cbegin(), points.cend());
	assert(max_points.size() == points[0].size());

	PlainTimeSeries<VectorSymbol> normalized_points(points.size());
	std::transform(points.cbegin(), points.cend(), begin(normalized_points),
				   std::bind(std::minus<VectorSymbol>(), _1, min_points));

	std::vector<VectorDouble> desample_table(max_points - min_points + 1);
	for (size_t i = 0; i < desample_table.size(); ++i) {
	  desample_table[i] = i + min_point;
	}

	Preprocessed_tseries<Double, Symbol> to_return(normalized_points);
	to_return.copy_preprocessing_info_from(points);
	to_return.set_desample_table(desample_table);
	to_return.set_sampling_alphabet(max_point - min_point + 1);

	return to_return;*/

	throw NotImplementedError("Transform for VectorSymbol is not implemented");
}

[[maybe_unused]]
VectorDouble
Sampler<VectorSymbol>::InverseTransform(Symbol, const Preproc_info<VectorDouble> &)
{
	throw NotImplementedError("InverseTransform for VectorSymbol is not implemented");
}

void CheckBase(size_t base)
{
	if (base < 2)
	{
		throw InvalidBaseError("Trying to convert a number using invalid base");
	}
}

Symbol ConvertNumberToDec(const VectorSymbol &number, size_t base)
{
	CheckBase(base);

	if (number.size() == 0)
	{
		throw EmptyInputError("Trying to convert an empty number");
	}

	Symbol to_return = 0;
	Symbol base_power = 1;
	for (size_t i = 0; i < number.size(); ++i)
	{
		if (number[i] >= base)
		{
			throw InvalidDigitError("A digit is greater or equal to the base");
		}

		to_return += base_power * number[i];
		base_power *= base;
	}

	return to_return;
}

VectorSymbol ConvertDecToNumber(Symbol s, size_t base)
{
	CheckBase(base);

	if (s == 0)
	{
		return {0};
	}

	std::vector<Symbol> result;
	while (s != 0)
	{
		result.push_back(s % base);
		s /= base;
	}

	return VectorSymbol(result.data(), result.size());
}
} // itp
