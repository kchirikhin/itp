#ifndef ITP_COMPRESSION_PREDICTION_H_INCLUDED_
#define ITP_COMPRESSION_PREDICTION_H_INCLUDED_

#include "Compnames.h"
#include "Compressors.h"
#include "PredictorSubtypes.h"

#include <algorithm>

namespace itp
{

template<typename T>
class CodeLengthsComputer
{
public:
	using Trajectories = ICompressor::Continuations;

	explicit CodeLengthsComputer(CompressorsFacadePtr compressors);
	virtual ~CodeLengthsComputer() = default;

	virtual ContinuationsDistribution<T> ComputeContinuationsDistribution(
		const PreprocessedTimeSeries<T, Symbol>& history,
		size_t length_of_continuation,
		const CompressorNames& compressor_names,
		const Trajectories& possible_continuations) const;

	virtual ContinuationsDistribution<T> ComputeContinuationsDistribution(
		const PreprocessedTimeSeries<T, Symbol>& history,
		size_t length_of_continuation,
		const CompressorNames& compressor_names) const;

private:
	CompressorsFacadePtr compressors_;
	static constexpr size_t bits_in_byte_ = 8;
};

template<typename T>
using CodeLengthsComputerPtr = std::shared_ptr<CodeLengthsComputer<T>>;

template<typename OrigType, typename NewType>
class CompressionBasedPredictor : public DistributionPredictor<OrigType, NewType>
{
public:
	explicit CompressionBasedPredictor(size_t difference_order = 0);
	explicit CompressionBasedPredictor(WeightsGeneratorPtr weights_generator, size_t difference_order = 0);

	ContinuationsDistribution<OrigType> Predict(
		PreprocessedTimeSeries<OrigType, NewType> history,
		size_t horizont,
		const CompressorNamesVec& compressor_groups) const final;

	void SetDifferenceOrder(size_t n);
	size_t GetDifferenceOrder() const;

protected:
	virtual ContinuationsDistribution<OrigType> ObtainCodeProbabilities(
		const PreprocessedTimeSeries<OrigType, NewType>& history,
		size_t horizont,
		const CompressorNames& compressor_names) const = 0;

private:
	WeightsGeneratorPtr weights_generator_;
	size_t difference_order_;
};

template<typename DoubleT>
class MultialphabetDistributionPredictor : public CompressionBasedPredictor<DoubleT, DoubleT>
{
public:
	MultialphabetDistributionPredictor() = delete;
	MultialphabetDistributionPredictor(
		CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
		SamplerPtr<DoubleT> sampler,
		size_t max_q,
		size_t difference = 0);

	ContinuationsDistribution<DoubleT> ObtainCodeProbabilities(
		const PreprocessedTimeSeries<DoubleT, DoubleT>& ts,
		size_t horizont,
		const CompressorNames& compressor_names) const override;

private:
	CodeLengthsComputerPtr<DoubleT> codes_lengths_computer_;
	SamplerPtr<DoubleT> sampler_;
	size_t log2_max_partition_cardinality_;
	WeightsGeneratorPtr partitions_weights_gen_;
};

template<typename OrigType, typename NewType>
class SingleAlphabetDistributionPredictor : public CompressionBasedPredictor<OrigType, NewType>
{
public:
	SingleAlphabetDistributionPredictor() = delete;
	explicit SingleAlphabetDistributionPredictor(CodeLengthsComputerPtr<OrigType>, size_t = 0);

protected:
	ContinuationsDistribution<OrigType> ObtainCodeProbabilities(
		const PreprocessedTimeSeries<OrigType, NewType>&,
		size_t,
		const CompressorNames&) const override final;
	virtual PreprocessedTimeSeries<OrigType, Symbol> Sample(const PreprocessedTimeSeries<OrigType, NewType>&) const = 0;

private:
	CodeLengthsComputerPtr<OrigType> codes_lengths_computer_;
};

template<typename DoubleT>
class RealDistributionPredictor : public SingleAlphabetDistributionPredictor<DoubleT, DoubleT>
{
public:
	RealDistributionPredictor() = delete;
	RealDistributionPredictor(
		CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
		SamplerPtr<DoubleT> sampler,
		size_t partition_cardinality,
		size_t difference = 0);

protected:
	PreprocessedTimeSeries<DoubleT, Symbol> Sample(const PreprocessedTimeSeries<DoubleT, DoubleT>& history) const override;

private:
	SamplerPtr<DoubleT> sampler_;
	size_t partition_cardinality_;
};

template<typename DoubleT, typename SymbolT>
class DiscreteDistributionPredictor : public SingleAlphabetDistributionPredictor<DoubleT, SymbolT>
{
public:
	DiscreteDistributionPredictor() = delete;
	DiscreteDistributionPredictor(
		CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
		SamplerPtr<SymbolT> sampler,
		size_t difference = 0);

protected:
	PreprocessedTimeSeries<DoubleT, Symbol> Sample(const PreprocessedTimeSeries<DoubleT, SymbolT>& history) const override;

private:
	SamplerPtr<SymbolT> sampler_;
};

template<typename T>
CodeLengthsComputer<T>::CodeLengthsComputer(CompressorsFacadePtr compressors)
	: compressors_{std::move(compressors)}
{
	// DO NOTHING
}

template<typename T>
ContinuationsDistribution<T> CodeLengthsComputer<T>::ComputeContinuationsDistribution(
	const PreprocessedTimeSeries<T, Symbol>& history,
	size_t length_of_continuation,
	const CompressorNames& compressor_names,
	const Trajectories& possible_continuations) const
{
	const auto alphabet = history.GetSamplingAlphabet();
	assert(length_of_continuation <= 100);
	assert(alphabet > 0);

	compressors_->SetAlphabetDescription({0, static_cast<Symbol>(alphabet - 1)});

	ContinuationsDistribution<T> result(
		std::begin(possible_continuations),
		std::end(possible_continuations),
		std::begin(compressor_names),
		std::end(compressor_names));

	const auto& plain_time_series = history.to_plain_tseries();
	for (size_t i = 0; i < result.FactorsSize(); ++i)
	{
		const auto code_lengths = compressors_->CompressContinuations(
			compressor_names[i],
			plain_time_series,
			possible_continuations);

		for (size_t j = 0; j < std::size(possible_continuations); ++j)
		{
			result(possible_continuations[j], compressor_names[i]) = code_lengths[j];
		}
	}

	return result;
}

template<typename T>
ContinuationsDistribution<T> CodeLengthsComputer<T>::ComputeContinuationsDistribution(
	const PreprocessedTimeSeries<T, Symbol>& history,
	size_t length_of_continuation,
	const CompressorNames& compressor_names) const
{
	const auto alphabet = history.GetSamplingAlphabet();
	assert(0 < alphabet);

	std::vector<Continuation<Symbol>> possible_continuations;
	Continuation<Symbol> continuation(alphabet, length_of_continuation);
	for (size_t i = 0; i < pow(alphabet, length_of_continuation); ++i)
	{
		possible_continuations.push_back(continuation++);
	}

	return ComputeContinuationsDistribution(history, length_of_continuation, compressor_names, possible_continuations);
}

template<typename OrigType, typename NewType>
ContinuationsDistribution<OrigType> CompressionBasedPredictor<OrigType, NewType>::Predict(
	PreprocessedTimeSeries<OrigType, NewType> history,
	size_t horizont,
	const CompressorNamesVec& compressor_groups) const
{
	auto differentized_history = DiffN(history, difference_order_);
	const auto distinct_single_compressors = FindAllDistinctNames(compressor_groups);
	auto code_probabilities_result = ObtainCodeProbabilities(
		differentized_history,
		horizont,
		distinct_single_compressors);
	FormGroupForecasts(code_probabilities_result, compressor_groups, weights_generator_);
	return ToProbabilities(code_probabilities_result);
}

template<typename DoubleT>
MultialphabetDistributionPredictor<DoubleT>::MultialphabetDistributionPredictor(
	CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
	SamplerPtr<DoubleT> sampler,
	size_t max_q,
	size_t difference_order)
	: CompressionBasedPredictor<DoubleT, DoubleT>{difference_order}
	, codes_lengths_computer_{codes_lengths_computer}
	, sampler_{sampler}
	, partitions_weights_gen_{std::make_shared<CountableWeightsGenerator>()}
{
	assert(codes_lengths_computer != nullptr);
	assert(sampler != nullptr);
	assert(max_q != 0);
	assert(IsPowerOfTwo(max_q));
	log2_max_partition_cardinality_ = log2(max_q);
}

template<typename DoubleT>
ContinuationsDistribution<DoubleT> MultialphabetDistributionPredictor<DoubleT>::ObtainCodeProbabilities(
	const PreprocessedTimeSeries<DoubleT, DoubleT>& history,
	size_t horizont,
	const CompressorNames& compressor_names) const
{
	size_t N = log2_max_partition_cardinality_;
	std::vector<ContinuationsDistribution<DoubleT>> tables(N);
	std::vector<size_t> alphabets(N);
	for (size_t i = 0; i < N; ++i)
	{
		auto sampled_ts = sampler_->Transform(history, static_cast<size_t>(pow(2, i + 1)));

		// In the vector case it will differ from 2^(i+1)!
		alphabets[i] = sampled_ts.GetSamplingAlphabet();
		tables[i] = codes_lengths_computer_->ComputeContinuationsDistribution(sampled_ts, horizont, compressor_names);
		tables[i].CopyPreprocessingInfoFrom(sampled_ts);
	}

	auto message_length = history.size() + horizont;
	for (size_t i = 0; i < N; ++i)
	{
		AddValueToEach(begin(tables[i]), end(tables[i]), (N - i - 1) * message_length);
	}
	auto global_minimal_code_length
		= MinValueOfAllTables<typename decltype(tables)::const_iterator, HighPrecDouble>(
			tables.cbegin(),
			tables.cend());
	for (auto& table : tables)
	{
		AddValueToEach(begin(table), end(table), -global_minimal_code_length);
		ToCodeProbabilities(begin(table), end(table));
	}

	auto table = Merge(tables, alphabets, partitions_weights_gen_->Generate(N));
	table.CopyPreprocessingInfoFrom(tables.back());

	return table;
}

template<typename DoubleT>
RealDistributionPredictor<DoubleT>::RealDistributionPredictor(
	CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
	SamplerPtr<DoubleT> sampler,
	size_t partition_cardinality,
	size_t difference_order)
	: SingleAlphabetDistributionPredictor<DoubleT, DoubleT>{codes_lengths_computer, difference_order}
	, sampler_{sampler}
	, partition_cardinality_{partition_cardinality}
{
	// DO NOTHING
}

template<typename DoubleT>
PreprocessedTimeSeries<DoubleT, itp::Symbol> RealDistributionPredictor<DoubleT>::Sample(
	const PreprocessedTimeSeries<DoubleT, DoubleT>& history) const
{
	auto sampling_result = sampler_->Transform(history, partition_cardinality_);
	return sampling_result;
}

template<typename DoubleT, typename SymbolT>
DiscreteDistributionPredictor<DoubleT, SymbolT>::DiscreteDistributionPredictor(
	CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
	SamplerPtr<SymbolT> sampler,
	size_t difference_order)
	: SingleAlphabetDistributionPredictor<DoubleT, SymbolT>{codes_lengths_computer, difference_order}
	, sampler_{sampler}
{
	// DO NOTHING
}

template<typename DoubleT, typename SymbolT>
PreprocessedTimeSeries<DoubleT, Symbol> DiscreteDistributionPredictor<DoubleT, SymbolT>::Sample(
	const PreprocessedTimeSeries<DoubleT, SymbolT>& history) const
{
	return sampler_->Transform(history);
}

template<typename OrigType, typename NewType>
ContinuationsDistribution<OrigType>
SingleAlphabetDistributionPredictor<OrigType, NewType>::ObtainCodeProbabilities(
	const PreprocessedTimeSeries<OrigType, NewType>& history,
	size_t horizont,
	const CompressorNames& compressor_names) const
{
	auto sampled_tseries = Sample(history);
	auto table = codes_lengths_computer_->ComputeContinuationsDistribution(sampled_tseries, horizont, compressor_names);
	auto min = *std::min_element(begin(table), end(table));
	AddValueToEach(begin(table), end(table), -min);
	ToCodeProbabilities(begin(table), end(table));
	table.CopyPreprocessingInfoFrom(sampled_tseries);

	return table;
}

template<typename OrigType, typename NewType>
CompressionBasedPredictor<OrigType, NewType>::CompressionBasedPredictor(size_t difference_order)
	: CompressionBasedPredictor<OrigType, NewType>{std::make_shared<WeightsGenerator>(), difference_order}
{
	// DO NOTHING
}

template<typename OrigType, typename NewType>
CompressionBasedPredictor<OrigType, NewType>::CompressionBasedPredictor(
	WeightsGeneratorPtr weights_generator,
	size_t difference_order)
	: weights_generator_{weights_generator}
	, difference_order_{difference_order}
{
	// DO NOTHING
}

template<typename OrigType, typename NewType>
void CompressionBasedPredictor<OrigType, NewType>::SetDifferenceOrder(size_t n)
{
	difference_order_ = n;
}

template<typename OrigType, typename NewType>
size_t CompressionBasedPredictor<OrigType, NewType>::GetDifferenceOrder() const
{
	return difference_order_;
}

template<typename OrigType, typename NewType>
SingleAlphabetDistributionPredictor<OrigType, NewType>::SingleAlphabetDistributionPredictor(
	CodeLengthsComputerPtr<OrigType> codes_lengths_computer,
	size_t)
	: codes_lengths_computer_{codes_lengths_computer}
{
	// DO NOTHING
}

} // namespace itp

#endif // ITP_COMPRESSION_PREDICTION_H_INCLUDED_
