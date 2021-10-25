#ifndef ITP_PREDICTOR_SUBTYPES_H_INCLUDED_
#define ITP_PREDICTOR_SUBTYPES_H_INCLUDED_

#include "dtypes.h"
#include "itp_exceptions.h"
#include "sampler.h"
#include "ttransformations.h"

#include <iterator>
#include <iostream>
#include <memory>
#include <cmath>

// Interface of the library.
namespace itp {
template
template <typename T>
std::ostream& operator << (std::ostream &ost, const ContinuationsDistribution<T> &table);

inline bool IsPowerOfTwo(std::size_t n) {
  return (n > 0 && ((n & (n - 1)) == 0));
}
  
template <typename OrigType, typename NewType>
class Distribution_predictor {
  // static_assert(std::is_arithmetic<NewType>::value, "NewType should be an arithmetic type");
 public:
  virtual ~Distribution_predictor() = default;

  virtual ContinuationsDistribution<OrigType> Predict(Preprocessed_tseries<OrigType, NewType> history,
                                                      size_t horizont, const std::vector<Names> &compressors) const = 0;
};

template <typename OrigType, typename NewType>
using Distribution_predictor_ptr = std::shared_ptr<Distribution_predictor<OrigType, NewType>>;

template <typename OrigType, typename NewType>
class Pointwise_predictor {
  // static_assert(std::is_arithmetic<NewType>::value, "NewType should be an arithmetic type");
 public:
  virtual ~Pointwise_predictor() = default;

  virtual Forecast<OrigType> Predict(Preprocessed_tseries<OrigType, NewType> history, size_t horizont,
                                     const std::vector<Names> &compressors) const = 0;
};

template <typename OrigType, typename NewType>
using Pointwise_predictor_ptr = std::shared_ptr<Pointwise_predictor<OrigType, NewType>>;

template <typename OrigType, typename NewType>
class Basic_pointwise_predictor : public Pointwise_predictor<OrigType, NewType> {
 public:
  Basic_pointwise_predictor(Distribution_predictor_ptr<OrigType, NewType> distribution_predictor);
  Forecast<OrigType> Predict(Preprocessed_tseries<OrigType, NewType> history, size_t horizont,
                              const std::vector<Names> &compressors) const override;
 private:
  Distribution_predictor_ptr<OrigType, NewType> distribution_predictor;
};

/**
 * Decorator pattern implementation.
 *
 */
template <typename OrigType, typename NewType>
class Sparse_predictor : public Pointwise_predictor<OrigType, NewType> {
 public:
  Sparse_predictor(Pointwise_predictor_ptr<OrigType, NewType> pointwise_predictor, size_t sparse);

  Forecast<OrigType> Predict(Preprocessed_tseries<OrigType, NewType> history, size_t horizont,
                              const std::vector<Names> &compressors) const override final;
 private:
  Pointwise_predictor_ptr<OrigType, NewType> pointwise_predictor;
  size_t sparse;
};

template <typename Forward_iterator, typename New_value>
New_value min_value_of_all_tables(Forward_iterator first, Forward_iterator last);

template <typename Forward_iterator, typename T>
void add_value_to_each(Forward_iterator first, Forward_iterator last, T value);

} // of itp

template <typename T>
std::ostream & itp::operator << (std::ostream &ost, const ContinuationsDistribution<T> &table) {
  ost << "-\t";
  for (const auto &compressor : table.get_factors()) {
    ost << compressor << '\t';
  }
  ost << '\n';
  for (const auto &continuation : table.get_index()) {
    ost << continuation << '\t';
    for (const auto &compressor : table.get_factors()) {
      ost << table(continuation, compressor) << '\t';
    }
    ost << '\n';
  }

  return ost;
}

template <typename OrigType, typename NewType>
itp::Forecast<OrigType> itp::Sparse_predictor<OrigType, NewType>::Predict(Preprocessed_tseries<OrigType, NewType> history, size_t horizont, const std::vector<Names> &compressors) const {
  std::vector<Forecast<OrigType>> results(sparse);
  size_t sparsed_horizont = ceil(horizont / static_cast<double>(sparse));
  for (size_t i = 0; i < sparse; ++i) {
	Preprocessed_tseries<OrigType, NewType> sparse_ts_data;
	sparse_ts_data.copy_preprocessing_info_from(history);
    for (size_t j = i; j < history.size(); j += sparse) {
      sparse_ts_data.push_back(history[j]);
    }
    results[i] = pointwise_predictor->Predict(sparse_ts_data, sparsed_horizont, compressors);
  }

  Forecast<OrigType> result;
  Forecast<OrigType> full_first_steps =
      pointwise_predictor->Predict(history, sparsed_horizont, compressors);
  for (size_t i = 0; i < sparsed_horizont; ++i) {
    for (const auto &compressor : full_first_steps.get_index()) {
      result(compressor, i) = full_first_steps(compressor, i);
    }
  }

  for (size_t i = 0; i < sparsed_horizont; ++i) {
    for (size_t j = 0; j < sparse; ++j) {
      if ((i*sparse+j >= sparsed_horizont) && (i*sparse+j < horizont)) {
        for (const auto compressor : results[j].get_index()) {
          result(compressor, i*sparse+j) = results[j](compressor, i);
        }
      }
    }
  }
  return result;
}

template <typename OrigType, typename NewType>
itp::Basic_pointwise_predictor<OrigType, NewType>::Basic_pointwise_predictor(Distribution_predictor_ptr<OrigType, NewType> distribution_predictor)
    : distribution_predictor {distribution_predictor} {
  // DO NOTHING
}

template <typename OrigType, typename NewType>
itp::Forecast<OrigType> itp::Basic_pointwise_predictor<OrigType, NewType>::Predict(Preprocessed_tseries<OrigType, NewType> ts, size_t horizont,
                                                                                      const std::vector<Names> &compressors) const {
  auto distribution = distribution_predictor->Predict(ts, horizont, compressors);
  auto forecasts = to_pointwise_forecasts(distribution, horizont);
  integrate(forecasts);
  return forecasts;
}

template <typename OrigType, typename NewType>
itp::Sparse_predictor<OrigType, NewType>::Sparse_predictor(Pointwise_predictor_ptr<OrigType, NewType> pointwise_predictor, size_t sparse)
    : pointwise_predictor(pointwise_predictor), sparse(sparse) {
  assert(pointwise_predictor != nullptr);
}

template <typename Forward_iterator, typename New_value>
New_value itp::min_value_of_all_tables(Forward_iterator first, Forward_iterator last) {
  New_value global_minimum {-1};
  while (first != last) {
    auto local_minimum = *std::min_element(begin(*first), end(*first));
    ++first;
    if ((local_minimum < global_minimum) || (global_minimum < 0)) {
      global_minimum = static_cast<New_value>(local_minimum);
    }
  }

  return global_minimum;
}

template <typename Forward_iterator, typename T>
void itp::add_value_to_each(Forward_iterator first, Forward_iterator last, T value) {
  while (first != last) {
    *first += value;
    ++first;
  }
}

#endif // ITP_PREDICTOR_SUBTYPES_H_INCLUDED_
