/**
 * @file   builders.h
 * @author Konstantin <user10101@user10101-Satellite-L855>
 * @date   Fri Jun  8 21:26:30 2018
 *
 * @brief  Contains auxilary functions and classes to facilitate creation of algorithm with
 * specific parameters.
 *
 *
 */

#ifndef ITP_BUILDERS_H_INCLUDED_
#define ITP_BUILDERS_H_INCLUDED_

#include "../../src/CompressionPrediction.h"
#include "INonCompressionAlgorithm.h"

#include <functional>
#include <memory>

template <typename OutType, typename InType>
class Forecasting_algorithm {
  // static_assert(std::is_arithmetic<T>::value, "T should be an arithmetic type");
 public:
	explicit Forecasting_algorithm(itp::CompressorsFacadePtr compressors);

  std::map<std::string, std::vector<OutType>> operator () (
  		const std::vector<InType> &time_series,
  		const itp::Names &compressors_groups,
  		size_t horizon,
  		size_t difference,
  		int sparse);

 protected:

  /**
   * Factory method.
   *
   */
  virtual itp::Pointwise_predictor_ptr<OutType, InType> make_predictor(
  		itp::CodeLengthsComputerPtr<OutType> computer,
  		itp::SamplerPtr<InType> sampler,
  		size_t difference) const = 0;

  itp::CompressorsFacadePtr compressors_;
};

template <typename OutType, typename InType>
Forecasting_algorithm<OutType, InType>::Forecasting_algorithm(itp::CompressorsFacadePtr compressors)
	: compressors_{std::move(compressors)}
{
	// DO NOTHING
}

template <typename OutType, typename InType>
std::map<std::string, std::vector<OutType>> Forecasting_algorithm<OutType, InType>::operator () (
		const std::vector<InType> &time_series,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		int sparse) {
  auto computer = std::make_shared<itp::CodeLengthsComputer<OutType>>(compressors_);
  auto sampler = std::make_shared<itp::Sampler<InType>>();
  std::vector<itp::Names> compressors {
    itp::split_concatenated_names(compressors_groups)};
  itp::Pointwise_predictor_ptr<OutType, InType> pointwise_predictor = make_predictor(
  		computer,
  		sampler,
  		difference);
  if (sparse > 0) {
    pointwise_predictor = std::make_shared<itp::Sparse_predictor<OutType, InType>>(pointwise_predictor, sparse);
  }

  itp::Forecast<OutType> res = pointwise_predictor->Predict(itp::InitPreprocessedTs(time_series), horizon, compressors);
  std::map<std::string, std::vector<OutType>> ret;
  for (const auto &compressor : res.get_index()) {
    ret[compressor] = std::vector<OutType>(horizon);
    for (size_t i = 0; i < horizon; ++i) {
      ret[compressor][i] = res(compressor, i).point;
    }
  }

  return ret;
}

/**
 * Forecast originally discrete time series.
 *
 */
template <typename DoubleT, typename SymbolT>
class Forecasting_algorithm_discrete : public Forecasting_algorithm<DoubleT, SymbolT> {
public:
	using Forecasting_algorithm<DoubleT, SymbolT>::Forecasting_algorithm;

 protected:
  itp::Pointwise_predictor_ptr<DoubleT, SymbolT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<SymbolT> sampler,
  		size_t difference) const override;
};

template <typename DoubleT, typename SymbolT>
class Forecasting_algorithm_discrete_automation : public Forecasting_algorithm<DoubleT, SymbolT> {
public:
	using Forecasting_algorithm<DoubleT, SymbolT>::Forecasting_algorithm;

protected:
  itp::Pointwise_predictor_ptr<DoubleT, SymbolT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<SymbolT> sampler,
  		size_t difference) const override;
};

/**
 * Forecast originally real-valued time series with discretization (discretize with
 * only one partition cardinality).
 *
 */
template <typename DoubleT>
class Forecasting_algorithm_real : public Forecasting_algorithm<DoubleT, DoubleT> {
public:
	using Forecasting_algorithm<DoubleT, DoubleT>::Forecasting_algorithm;

	void set_quants_count(size_t n);

protected:
  itp::Pointwise_predictor_ptr<DoubleT, DoubleT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<DoubleT> sampler,
  		size_t difference) const override;

protected:
  size_t quants_count;
};

template <typename DoubleT>
class Forecasting_algorithm_real_automation : public Forecasting_algorithm_real<DoubleT> {
public:
	using Forecasting_algorithm<DoubleT, DoubleT>::Forecasting_algorithm;

protected:
  itp::Pointwise_predictor_ptr<DoubleT, DoubleT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<DoubleT> sampler,
  		size_t difference) const override;
};

template <typename DoubleT>
class Forecasting_algorithm_multialphabet : public Forecasting_algorithm_real<DoubleT> {
public:
	using Forecasting_algorithm_real<DoubleT>::Forecasting_algorithm_real;

protected:
  itp::Pointwise_predictor_ptr<DoubleT, DoubleT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<DoubleT> sampler,
  		size_t difference) const override;
};

template <typename DoubleT>
class Forecasting_algorithm_multialphabet_automation : public Forecasting_algorithm_real_automation<DoubleT> {
public:
	using Forecasting_algorithm<DoubleT, DoubleT>::Forecasting_algorithm;

protected:
  itp::Pointwise_predictor_ptr<DoubleT, DoubleT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<DoubleT> sampler,
  		size_t difference) const override;
};

template <typename DoubleT, typename SymbolT>
itp::Pointwise_predictor_ptr<DoubleT, SymbolT>
Forecasting_algorithm_discrete<DoubleT, SymbolT>::make_predictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<SymbolT> sampler,
		size_t difference) const {
  auto dpredictor = std::make_shared<itp::Discrete_distribution_predictor<DoubleT, SymbolT>>(computer, sampler,
                                                                                             difference);
  return std::make_shared<itp::Basic_pointwise_predictor<DoubleT, SymbolT>>(dpredictor);
}

template <typename DoubleT>
void Forecasting_algorithm_real<DoubleT>::set_quants_count(size_t n) {
  quants_count = n;
}

template <typename DoubleT>
itp::Pointwise_predictor_ptr<DoubleT, DoubleT>
Forecasting_algorithm_real<DoubleT>::make_predictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<DoubleT> sampler,
		size_t difference) const {
  auto dpredictor = std::make_shared<itp::Real_distribution_predictor<DoubleT>>(computer, sampler, quants_count,
                                                                                difference);
  return std::make_shared<itp::Basic_pointwise_predictor<DoubleT, DoubleT>>(dpredictor);
}

template <typename DoubleT>
itp::Pointwise_predictor_ptr<DoubleT, DoubleT>
Forecasting_algorithm_multialphabet<DoubleT>::make_predictor(
		itp::CodeLengthsComputerPtr<DoubleT> computer,
		itp::SamplerPtr<DoubleT> sampler,
		size_t difference) const {
  auto dpredictor = std::make_shared<itp::Multialphabet_distribution_predictor<DoubleT>>(computer, sampler,
                                                                                         Forecasting_algorithm_real<DoubleT>::quants_count,
                                                                                         difference);
  return std::make_shared<itp::Basic_pointwise_predictor<DoubleT, DoubleT>>(dpredictor);
}

#endif // ITP_BUILDERS_H_INCLUDED_
