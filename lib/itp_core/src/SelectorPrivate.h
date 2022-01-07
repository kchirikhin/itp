//
// Created by kon on 26.10.2021.
//

#ifndef ITP_CORE_SELECTOR_PRIVATE_H_INCLUDED_
#define ITP_CORE_SELECTOR_PRIVATE_H_INCLUDED_

#include <Selector.h>

#include "../../src/Compressors.h"
#include "../../src/Sampler.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>

namespace itp::evaluation
{

/**
 * Allows to get code lengths for the specified series by compressing it with the specified compressors.
 * @tparam T Data type of the series to compress.
 */
template<typename T>
class CodeLengthEvaluator
{
public:
	/**
	 * Inject interface to call compressors.
	 * @param compressors An interface, which allows to call a compressor by its name.
	 */
	explicit CodeLengthEvaluator(CompressorsFacadePtr compressors)
			: compressors_{std::move(compressors)}
	{
		// DO NOTHING
	}

	/**
	 * Compresses the specified series with the specified compressors and returns the obtained code lengths. All
	 * valid preliminary transformations are applied.
	 * @param history The series (data) to compress.
	 * @param compressor_names Names of compressors to use. The compressors must be available via CompressorsFacade object
	 * injected via constructor.
	 * @param difference Order of difference.
	 * @param quanta_count For a real-valued time series, all specified quanta count will be considered at the same
	 * time. For a discrete series this parameter is ignored.
	 * @return For each compressor its best code length (in the form of <name -> length> mapping).
	 */
	std::unordered_map<std::string, size_t> Evaluate(const std::vector<T>& history,
													 const CompressorNames& compressor_names, size_t difference,
													 const std::vector<size_t>& quanta_count);

private:
	CompressorsFacadePtr compressors_;
};

/**
 * Computes n-th adjacent difference of a sequence,
 * @tparam T Type of elements of the sequence.
 * @param vec The sequence for computations.
 * @param n Number of differences to take.
 * @return Computes series of adjacent differences.
 */
template<typename T>
std::vector<T> diff_n(std::vector<T> vec, size_t n = 1)
{
	if (vec.size() <= n)
	{
		return {};
	}

	for (size_t i = 1; i <= n; ++i)
	{
		for (size_t j = 0; j < vec.size() - i; ++j)
		{
			vec[j] = vec[j + 1] - vec[j];
		}
	}
	vec.erase(std::end(vec) - n, std::end(vec));

	return vec;
}

template<typename T>
class QuantifiedVector : public std::vector<T>
{
public:
	using std::vector<T>::vector;

	QuantifiedVector(const std::vector<T>& vec)
			: std::vector<T>(std::cbegin(vec), std::cend(vec))
	{
	}

	void SetAlphabetSize(size_t max_letter)
	{
		max_letter_ = max_letter;
	}

	[[nodiscard]]
	size_t GetAlphabetSize() const
	{
		return max_letter_;
	}

private:
	size_t max_letter_ = 256;
};

template<typename T>
class SampledSeriesStorageBase
{
public:
	using value_type = QuantifiedVector<Symbol>;
	using reference = QuantifiedVector<Symbol>&;
	using const_reference = const value_type&;
	using difference_type = ptrdiff_t;
	using size_type = size_t;

	[[nodiscard]]
	std::size_t size() const
	{
		return quantified_series_.size();
	}

	class const_iterator
	{
	public:
		using value_type = SampledSeriesStorageBase<T>::value_type;
		using reference = SampledSeriesStorageBase<T>::reference;
		using const_reference = SampledSeriesStorageBase<T>::const_reference;
		using difference_type = SampledSeriesStorageBase<T>::difference_type;
		using size_type = SampledSeriesStorageBase<T>::size_type;

		const_iterator() = default;

		explicit const_iterator(const SampledSeriesStorageBase<T>* storage, size_t current_pos = 0)
				: storage_{storage}, current_pos_{current_pos}
		{
			// DO NOTHING
		}

		const_reference operator*() const
		{
			return storage_->GetTimeSeries(current_pos_);
		}

		const value_type* operator->() const
		{
			return &storage_->GetTimeSeries(current_pos_);
		}

		bool operator==(const const_iterator& other) const
		{
			return current_pos_ == other.current_pos_;
		}

		bool operator!=(const const_iterator& other) const
		{
			return current_pos_ != other.current_pos_;
		}

		const_iterator& operator++()
		{
			++current_pos_;
			return *this;
		}

		const_iterator operator++(int)
		{
			const auto to_return = *this;
			++current_pos_;
			return to_return;
		}

	private:
		const SampledSeriesStorageBase<T>* storage_ = nullptr;
		size_t current_pos_ = 0;
	};

	const_iterator cbegin() const
	{
		return const_iterator{this};
	}

	const_iterator begin() const
	{
		return const_iterator{this};
	}

	const_iterator cend() const
	{
		return const_iterator{this, quantified_series_.size()};
	}

	const_iterator end() const
	{
		return const_iterator{this, quantified_series_.size()};
	}

protected:
	explicit SampledSeriesStorageBase(const std::vector<T>& original_series);

	SampledSeriesStorageBase(const std::vector<T>& original_series, const std::vector<size_t>& quanta_counts);

private:
	[[nodiscard]]
	const QuantifiedVector<Symbol>& GetTimeSeries(size_t index) const;

	Sampler<T> sampler_;
	std::vector<QuantifiedVector<Symbol>> quantified_series_;
};

template<typename T>
SampledSeriesStorageBase<T>::SampledSeriesStorageBase(const std::vector<T>& original_series)
{
	const auto quantified_series = sampler_.Transform(InitPreprocessedTs(original_series));
	quantified_series_.push_back(quantified_series.to_plain_tseries());
	quantified_series_.back().SetAlphabetSize(quantified_series.get_sampling_alphabet());
}

template<typename T>
SampledSeriesStorageBase<T>::SampledSeriesStorageBase(const std::vector<T>& original_series,
													  const std::vector<size_t>& quanta_counts)
{
	for (const auto quanta_count : quanta_counts)
	{
		auto quantified_series = sampler_.Transform(InitPreprocessedTs(original_series), quanta_count);
		quantified_series_.push_back(quantified_series.to_plain_tseries());
		quantified_series_.back().SetAlphabetSize(quantified_series.get_sampling_alphabet());
	}
}

template<typename T>
const QuantifiedVector<Symbol>& SampledSeriesStorageBase<T>::GetTimeSeries(size_t index) const
{
	return quantified_series_[index];
}

template<typename T>
class SampledSeriesStorage;

template<>
class SampledSeriesStorage<Double> : public SampledSeriesStorageBase<Double>
{
public:
	explicit SampledSeriesStorage(const std::vector<Double>& ts, const std::vector<size_t>& quanta_counts)
			: SampledSeriesStorageBase{ts, quanta_counts}
	{
		// DO NOTHING
	}
};

template<>
class SampledSeriesStorage<VectorDouble> : public SampledSeriesStorageBase<VectorDouble>
{
public:
	explicit SampledSeriesStorage(const std::vector<VectorDouble>& ts, const std::vector<size_t>& quanta_counts)
			: SampledSeriesStorageBase{ts, quanta_counts}
	{
		// DO NOTHING
	}
};

template<>
class SampledSeriesStorage<Symbol> : public SampledSeriesStorageBase<Symbol>
{
public:
	explicit SampledSeriesStorage(const std::vector<Symbol>& ts, const std::vector<size_t>& = {})
			: SampledSeriesStorageBase{ts}
	{
		// DO NOTHING
	}
};

template<>
class SampledSeriesStorage<VectorSymbol> : public SampledSeriesStorageBase<VectorSymbol>
{
public:
	explicit SampledSeriesStorage(const std::vector<VectorSymbol>& ts, const std::vector<size_t>& = {})
			: SampledSeriesStorageBase{ts}
	{
		// DO NOTHING
	}
};

template<typename T>
std::vector<size_t> ComputeCorrections(const std::vector<size_t>& quanta_counts, const size_t ts_length)
{
	if (quanta_counts.empty())
	{
		return {};
	}

	std::vector<size_t> to_return(quanta_counts.size());
	const auto max_quanta_count = *std::max_element(std::cbegin(quanta_counts), std::cend(quanta_counts));
	const auto max_quanta_count_log = static_cast<size_t>(log2(static_cast<double>(max_quanta_count)));
	std::transform(std::cbegin(quanta_counts), std::cend(quanta_counts), std::begin(to_return), [=](size_t quanta_count)
	{
		return (max_quanta_count_log - static_cast<size_t>(log2(static_cast<double>(quanta_count)))) * ts_length;
	});

	return to_return;
}

template<>
std::vector<size_t> ComputeCorrections<Symbol>(const std::vector<size_t>& quanta_counts, const size_t);

template<>
std::vector<size_t> ComputeCorrections<VectorSymbol>(const std::vector<size_t>& quanta_counts, const size_t);

template<typename T>
void CheckQuantaCounts(const std::vector<size_t>& quanta_counts)
{
	if (quanta_counts.empty())
	{
		throw SelectorError("quanta_counts is empty");
	}
}

template<>
void CheckQuantaCounts<Symbol>(const std::vector<size_t>&);

template<>
void CheckQuantaCounts<VectorSymbol>(const std::vector<size_t>&);

template<typename T>
std::unordered_map<std::string, size_t> CodeLengthEvaluator<T>::Evaluate(const std::vector<T>& history,
																		 const CompressorNames& compressor_names,
																		 const size_t difference,
																		 const std::vector<size_t>& quanta_counts)
{
	CheckQuantaCounts<T>(quanta_counts);

	const auto diff_history = diff_n(history, difference);
	const auto corrections = ComputeCorrections<T>(quanta_counts, diff_history.size());
	std::unordered_map<std::string, size_t> to_return;
	const SampledSeriesStorage<T> series_storage{diff_history, quanta_counts};
	for (const auto& compressor_name : compressor_names)
	{
		if (diff_history.empty())
		{
			to_return[compressor_name] = 0;
		}
		else
		{
			std::priority_queue<size_t, std::vector<size_t>, std::greater<>> code_lengths;
			auto correction = std::cbegin(corrections);
			for (const auto& series : series_storage)
			{
				compressors_->SetAlphabetDescription({0, static_cast<Symbol>(series.GetAlphabetSize() - 1)});
				code_lengths.push(
						compressors_->Compress(compressor_name, reinterpret_cast<const unsigned char*>(series.data()),
											   series.size() * sizeof(Symbol)) + *correction++);
			}
			to_return[compressor_name] = code_lengths.top();
		}
	}

	return to_return;
}

// std::for_each_n was not implemented in the libstdc++ at the moment of writing this code.
namespace ad_hoc
{

template<typename InputIt, typename Size, typename UnaryFunction>
InputIt for_each_n(InputIt first, Size n, UnaryFunction f)
{
	for (size_t i = 0; i < n; ++i, f(*first++));
	return first;
}

} // namespace ad_hoc

bool CompressionResultComparator(const std::pair<std::string, size_t>& lhs, const std::pair<std::string, size_t>& rhs);

itp::CompressorNames GetBestCompressors(
	const std::unordered_map<std::string, size_t>& results_of_computations,
	const size_t target_number);

} // itp::evaluation

#endif //ITP_CORE_SELECTOR_PRIVATE_H_INCLUDED_
