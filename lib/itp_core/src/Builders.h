/**
 * @file   builders.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Fri Jun  8 21:26:30 2018
 *
 * @brief  Contains auxilary functions and classes to facilitate creation of algorithm with
 * specific parameters.
 */

#ifndef ITP_BUILDERS_H_INCLUDED_
#define ITP_BUILDERS_H_INCLUDED_

#include "../include/itp_core/INonCompressionAlgorithm.h"
#include "CompressionPrediction.h"

#include <functional>
#include <memory>

template<typename OutType, typename InType>
class ForecastingAlgorithm
{
	// static_assert(std::is_arithmetic<T>::value, "T should be an arithmetic type");

public:
	explicit ForecastingAlgorithm(itp::CompressorsFacadePtr compressors);

	std::map<std::string, std::vector<OutType>> operator()(
		const std::vector<InType>& time_series,
		const itp::ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
		size_t horizon,
		size_t difference,
		int sparse);

protected:
	/**
	 * Factory method.
	 */
	virtual itp::PointwisePredictorPtr<OutType, InType> MakePredictor(
		itp::CodeLengthsComputerPtr<OutType> computer,
		itp::SamplerPtr<InType> sampler,
		size_t difference) const = 0;

	itp::CompressorsFacadePtr compressors_;
};

template<typename OutType, typename InType>
ForecastingAlgorithm<OutType, InType>::ForecastingAlgorithm(itp::CompressorsFacadePtr compressors)
	: compressors_{std::move(compressors)}
{
	// DO NOTHING
}

template<typename OutType, typename InType>
std::map<std::string, std::vector<OutType>> ForecastingAlgorithm<OutType, InType>::operator()(
	const std::vector<InType>& time_series,
	const itp::ConcatenatedCompressorNamesVec& concatenated_compressor_groups,
	size_t horizon,
	size_t difference,
	int sparse)
{
	auto computer = std::make_shared<itp::CodeLengthsComputer<OutType>>(compressors_);
	auto sampler = std::make_shared<itp::Sampler<InType>>();
	const auto compressor_groups = itp::SplitConcatenatedNames(concatenated_compressor_groups);
	itp::PointwisePredictorPtr<OutType, InType> pointwise_predictor = MakePredictor(computer, sampler, difference);
	if (sparse > 0)
	{
		pointwise_predictor = std::make_shared<itp::SparsePredictor<OutType, InType>>(pointwise_predictor, sparse);
	}

	itp::Forecast<OutType> res = pointwise_predictor->Predict(
		itp::InitPreprocessedTs(time_series),
		horizon,
		compressor_groups);
	std::map<std::string, std::vector<OutType>> ret;
	for (const auto& compressor : res.GetIndex())
	{
		ret[compressor] = std::vector<OutType>(horizon);
		for (size_t i = 0; i < horizon; ++i)
		{
			ret[compressor][i] = res(compressor, i).point;
		}
	}

	return ret;
}

/**
 * Forecast originally discrete time series.
 */
template<typename DoubleT, typename SymbolT>
class ForecastingAlgorithmDiscrete : public ForecastingAlgorithm<DoubleT, SymbolT>
{
public:
	using ForecastingAlgorithm<DoubleT, SymbolT>::ForecastingAlgorithm;

protected:
	itp::PointwisePredictorPtr<DoubleT, SymbolT> MakePredictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<SymbolT> sampler,
		size_t difference) const override;
};

template<typename DoubleT, typename SymbolT>
class ForecastingAlgorithmDiscreteAutomation : public ForecastingAlgorithm<DoubleT, SymbolT>
{
public:
	using ForecastingAlgorithm<DoubleT, SymbolT>::ForecastingAlgorithm;

protected:
	itp::PointwisePredictorPtr<DoubleT, SymbolT> MakePredictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<SymbolT> sampler,
		size_t difference) const override;
};

/**
 * Forecast originally real-valued time series with discretization (discretize with
 * only one partition cardinality).
 *
 */
template<typename DoubleT>
class ForecastingAlgorithmReal : public ForecastingAlgorithm<DoubleT, DoubleT>
{
public:
	using ForecastingAlgorithm<DoubleT, DoubleT>::ForecastingAlgorithm;

	void SetQuantaCount(size_t n);

protected:
	itp::PointwisePredictorPtr<DoubleT, DoubleT> MakePredictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<DoubleT> sampler,
		size_t difference) const override;

protected:
	size_t quanta_count_;
};

template<typename DoubleT>
class ForecastingAlgorithmRealAutomation : public ForecastingAlgorithmReal<DoubleT>
{
public:
	using ForecastingAlgorithm<DoubleT, DoubleT>::ForecastingAlgorithm;

protected:
	itp::PointwisePredictorPtr<DoubleT, DoubleT> MakePredictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<DoubleT> sampler,
		size_t difference) const override;
};

template<typename DoubleT>
class ForecastingAlgorithmMultialphabet : public ForecastingAlgorithmReal<DoubleT>
{
public:
	using ForecastingAlgorithmReal<DoubleT>::ForecastingAlgorithmReal;

protected:
	itp::PointwisePredictorPtr<DoubleT, DoubleT> MakePredictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<DoubleT> sampler,
		size_t difference) const override;
};

template<typename DoubleT>
class ForecastingAlgorithmMultialphabetAutomation : public ForecastingAlgorithmRealAutomation<DoubleT>
{
public:
	using ForecastingAlgorithm<DoubleT, DoubleT>::ForecastingAlgorithm;

protected:
	itp::PointwisePredictorPtr<DoubleT, DoubleT> MakePredictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<DoubleT> sampler,
		size_t difference) const override;
};

template<typename DoubleT, typename SymbolT>
itp::PointwisePredictorPtr<DoubleT, SymbolT> ForecastingAlgorithmDiscrete<DoubleT, SymbolT>::MakePredictor(
	itp::CodeLengthsComputerPtr<DoubleT> computer,
	itp::SamplerPtr<SymbolT> sampler,
	size_t difference) const
{
	auto dpredictor = std::make_shared<itp::DiscreteDistributionPredictor<DoubleT, SymbolT>>(
		computer,
		sampler,
		difference);
	return std::make_shared<itp::BasicPointwisePredictor<DoubleT, SymbolT>>(dpredictor);
}

template<typename DoubleT>
void ForecastingAlgorithmReal<DoubleT>::SetQuantaCount(size_t n)
{
	quanta_count_ = n;
}

template<typename DoubleT>
itp::PointwisePredictorPtr<DoubleT, DoubleT> ForecastingAlgorithmReal<DoubleT>::MakePredictor(
	itp::CodeLengthsComputerPtr<DoubleT> computer,
	itp::SamplerPtr<DoubleT> sampler,
	size_t difference) const
{
	auto dpredictor = std::make_shared<itp::RealDistributionPredictor<DoubleT>>(
		computer,
		sampler,
		quanta_count_,
		difference);
	return std::make_shared<itp::BasicPointwisePredictor<DoubleT, DoubleT>>(dpredictor);
}

template<typename DoubleT>
itp::PointwisePredictorPtr<DoubleT, DoubleT> ForecastingAlgorithmMultialphabet<DoubleT>::MakePredictor(
	itp::CodeLengthsComputerPtr<DoubleT> computer,
	itp::SamplerPtr<DoubleT> sampler,
	size_t difference) const
{
	auto dpredictor = std::make_shared<itp::MultialphabetDistributionPredictor<DoubleT>>(
		computer,
		sampler,
		ForecastingAlgorithmReal<DoubleT>::quanta_count_,
		difference);
	return std::make_shared<itp::BasicPointwisePredictor<DoubleT, DoubleT>>(dpredictor);
}

#endif // ITP_BUILDERS_H_INCLUDED_
