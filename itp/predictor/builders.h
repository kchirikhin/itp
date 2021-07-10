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

#include "compression_prediction.h"

#include <functional>
#include <memory>

/**
 * Removes empty symbols from start and end of a line.
 *
 * @param line String in which empty symbols should be removed.
 * @param is_empty_char Function to determine which character is empty or not
 * (return not zero if empty).
 *
 * @return Trimmed line.
 */
std::string trim_line(const std::string &line, const std::function<int(int)> &is_empty_char);

template <typename OutType, typename InType>
class Forecasting_algorithm {
  // static_assert(std::is_arithmetic<T>::value, "T should be an arithmetic type");
 public:
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
};

template <typename OutType, typename InType>
std::map<std::string, std::vector<OutType>> Forecasting_algorithm<OutType, InType>::operator () (
		const std::vector<InType> &time_series,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		int sparse) {
  auto computer = std::make_shared<itp::CodeLengthsComputer<OutType>>();
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
 protected:
  itp::Pointwise_predictor_ptr<DoubleT, SymbolT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<SymbolT> sampler,
  		size_t difference) const override;
};

template <typename DoubleT, typename SymbolT>
class Forecasting_algorithm_discrete_automation : public Forecasting_algorithm<DoubleT, SymbolT> {
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
 protected:
  itp::Pointwise_predictor_ptr<DoubleT, DoubleT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<DoubleT> sampler,
  		size_t difference) const override;
};

template <typename DoubleT>
class Forecasting_algorithm_multialphabet : public Forecasting_algorithm_real<DoubleT> {
 protected:
  itp::Pointwise_predictor_ptr<DoubleT, DoubleT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<DoubleT> sampler,
  		size_t difference) const override;
};

template <typename DoubleT>
class Forecasting_algorithm_multialphabet_automation : public Forecasting_algorithm_real_automation<DoubleT> {
 protected:
  itp::Pointwise_predictor_ptr<DoubleT, DoubleT> make_predictor(
  		itp::CodeLengthsComputerPtr<DoubleT> computer,
  		itp::SamplerPtr<DoubleT> sampler,
  		size_t difference) const override;
};

void check_args(size_t horizont, size_t difference, size_t quants_count, int sparse);

std::map<std::string, std::vector<itp::Double>>
make_forecast_real(
		const std::vector<itp::Double> &time_series,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		size_t quanta_count,
		int sparse);

std::map<std::string, std::vector<itp::Double>>
make_forecast_multialphabet(
		const std::vector<double> &history,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		size_t max_quanta_count,
		int sparse);

std::map<std::string, std::vector<std::vector<double>>>
make_forecast_multialphabet_vec(
		const std::vector<std::vector<double>> &history,
		const itp::Names &compressors_groups,
		size_t horizon,
		size_t difference,
		size_t max_quanta_count,
		int sparse);

std::map<std::string, std::vector<itp::Double>>
make_forecast_discrete(
		const std::vector<itp::Symbol> &history,
		const std::vector<std::string> &compressors_groups,
		size_t horizon,
		size_t difference,
		int sparse);

std::map<std::string, std::vector<itp::VectorDouble>>
make_forecast_discrete_vec(
		const std::vector<itp::VectorSymbol> &history,
		const std::vector<std::string> &compressors_groups,
		size_t horizon,
		size_t difference,
		int sparse);

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

std::vector<itp::VectorDouble> Convert(const std::vector<std::vector<double>> &series);
std::vector<std::vector<double>> Convert(const std::vector<itp::VectorDouble> &res);

#endif // ITP_BUILDERS_H_INCLUDED_
