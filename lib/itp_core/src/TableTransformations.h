#ifndef ITP_TTRANSFORMATIONS_H_INCLUDED_
#define ITP_TTRANSFORMATIONS_H_INCLUDED_

#include "Compnames.h"
#include "Sampler.h"
#include "Types.h"

#include <cmath>

namespace itp
{

class WeightsGenerator
{
public:
	virtual ~WeightsGenerator() = default;

	virtual std::vector<Double> Generate(size_t n) const;
};

class CountableWeightsGenerator : public WeightsGenerator
{
public:
	virtual std::vector<Double> Generate(size_t n) const;
};

using WeightsGeneratorPtr = std::shared_ptr<WeightsGenerator>;

VectorDouble operator*(const VectorDouble& lhs, Double rhs);

template<typename T>
T ZeroInitialized(const SymbolsDistributions<T>& d);
template<>
Symbol ZeroInitialized<Symbol>(const SymbolsDistributions<Symbol>&);
template<>
Double ZeroInitialized<Double>(const SymbolsDistributions<Double>&);
template<>
VectorSymbol ZeroInitialized<VectorSymbol>(const SymbolsDistributions<VectorSymbol>& d);
template<>
VectorDouble ZeroInitialized<VectorDouble>(const SymbolsDistributions<VectorDouble>& d);

template<typename T>
T Mean(const SymbolsDistributions<T>& d, const typename SymbolsDistributions<T>::FactorType& compressor)
{
	auto sum = ZeroInitialized<T>(d);
	Sampler<T> sampler;
	for (auto interval_no : d.GetIndex())
	{
		sum += sampler.InverseTransform(interval_no, d) * d(interval_no, compressor);
	}

	return sum;
}

/**
 * Takes n-th difference of the specified time series.
 *
 * \param[in] x Time series to differentization.
 * \param[in] n Differentize the time series n times.
 *
 * @return Differentized time series.
 */
template<typename OrigType, typename NewType>
PreprocessedTimeSeries<OrigType, NewType> DiffN(const PreprocessedTimeSeries<OrigType, NewType>& x, size_t n)
{
	PreprocessedTimeSeries<OrigType, NewType> diff_x(x.cbegin(), x.cend());
	diff_x.CopyPreprocessingInfoFrom(x);

	for (size_t i = 1; i <= n; ++i)
	{
		diff_x.PushLastDiffValue(diff_x[diff_x.size() - i]);
		for (size_t j = 0; j < diff_x.size() - i; ++j)
		{
			diff_x[j] = diff_x[j + 1] - diff_x[j];
		}
	}
	diff_x.erase(diff_x.end() - n, diff_x.end());

	assert(diff_x.AppliedDiffCount() == n);

	return diff_x;
}

template<typename T>
void Integrate(Forecast<T>& forecast)
{
	T last_value;
	while (0 < forecast.AppliedDiffCount())
	{
		last_value = forecast.PopLastDiffValue();
		for (const auto& compressor : forecast.GetIndex())
		{
			forecast(compressor, 0).point += last_value;
			forecast(compressor, 0).left_border += last_value;
			forecast(compressor, 0).right_border += last_value;
			for (size_t j = 1; j < forecast.FactorsSize(); ++j)
			{
				forecast(compressor, j).point += forecast(compressor, j - 1).point;
				forecast(compressor, j).left_border += forecast(compressor, j - 1).left_border;
				forecast(compressor, j).right_border += forecast(compressor, j - 1).right_border;
			}
		}
	}
}

template<typename ForwardIterator>
inline void ToCodeProbabilities(ForwardIterator first, ForwardIterator last)
{
	HighPrecDouble base = 2.;
	while (first != last)
	{
		*first = bignums::pow(base, -(*first));
		++first;
	}
}

template<typename T>
void FormGroupForecasts(
	ContinuationsDistribution<T>& code_probabilities,
	const CompressorNamesVec& compressors_groups,
	WeightsGeneratorPtr weights_generator)
{
	for (const auto& group : compressors_groups)
	{
		if (group.size() > 1)
		{
			auto group_concatenated_name = ToConcatenatedCompressorNames(group);
			auto weights = weights_generator->Generate(group.size());
			for (const auto& continuation : code_probabilities.GetIndex())
			{
				code_probabilities(continuation, group_concatenated_name) = 0;
				for (size_t i = 0; i < group.size(); ++i)
				{
					code_probabilities(continuation, group_concatenated_name) += code_probabilities(
																					 continuation,
																					 group[i])
						* weights[i];
				}
			}
		}
	}
}

template<typename T>
ContinuationsDistribution<T> ToProbabilities(ContinuationsDistribution<T> code_probabilities)
{
	Double cumulated_sum;
	for (const auto& compressor : code_probabilities.GetFactors())
	{
		cumulated_sum = .0;
		for (const auto& continuation : code_probabilities.GetIndex())
		{
			cumulated_sum += static_cast<Double>(code_probabilities(continuation, compressor));
		}

		for (const auto& continuation : code_probabilities.GetIndex())
		{
			code_probabilities(continuation, compressor) /= cumulated_sum;
		}
	}

	return code_probabilities;
}

template<typename T>
ContinuationsDistribution<T> Merge(
	const std::vector<ContinuationsDistribution<T>>& tables,
	const std::vector<size_t>& alphabets,
	const std::vector<Double>& weights)
{
	assert(tables.size() == weights.size());
	assert(std::is_sorted(begin(alphabets), end(alphabets)));

	std::vector<size_t> steps(tables.size());
	auto maximal_alphabet = alphabets[alphabets.size() - 1];
	std::transform(
		begin(alphabets),
		end(alphabets),
		begin(steps),
		[&maximal_alphabet](size_t item) { return maximal_alphabet / item; });

	ContinuationsDistribution<T> result(tables[tables.size() - 1]);
	for (const auto& continuation : result.GetIndex())
	{
		for (const auto& compressor : result.GetFactors())
		{
			result(continuation, compressor) = .0;
			for (size_t i = 0; i < tables.size(); ++i)
			{
				result(continuation, compressor) += tables[i](continuation / steps[i], compressor) * weights[i];
			}
		}
	}
	result.CopyPreprocessingInfoFrom(tables.back());

	return result;
}

template<typename T>
Forecast<T> ToPointwiseForecasts(
	const ContinuationsDistribution<T>& table,
	size_t h,
	double confidence_probability = 0.95)
{
	Forecast<T> result;
	for (size_t i = 0; i < h; ++i)
	{
		SymbolsDistributions<T> d = CumulatedForStep(table, i);
		for (auto compressor : d.GetFactors())
		{
			result(compressor, i).point = Mean(d, compressor);
		}
	}
	result.CopyPreprocessingInfoFrom(table);

	return result;
}

template<typename T>
SymbolsDistributions<T> CumulatedForStep(const ContinuationsDistribution<T>& table, std::size_t step)
{
	assert(step <= 1000);

	SymbolsDistributions<T> result;
	for (const auto& continuation : table.GetIndex())
	{
		for (const auto& compressor : table.GetFactors())
		{
			result(continuation[step], compressor) = 0;
		}
	}

	for (const auto& continuation : table.GetIndex())
	{
		for (const auto& compressor : table.GetFactors())
		{
			result(continuation[step], compressor) += table(continuation, compressor);
		}
	}
	result.CopyPreprocessingInfoFrom(table);

	return result;
}

} // namespace itp

#endif // ITP_TTRANSFORMATIONS_H_INCLUDED_
