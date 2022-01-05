#ifndef ITP_TTRANSFORMATIONS_H_INCLUDED_
#define ITP_TTRANSFORMATIONS_H_INCLUDED_

#include "Types.h"
#include "Compnames.h"
#include "Sampler.h"

#include <cmath>

namespace itp {
class Weights_generator {
 public:
  virtual ~Weights_generator() = default;

  virtual std::vector<Double> generate(size_t n) const;
};

class Countable_weights_generator : public Weights_generator {
 public:
  virtual std::vector<Double> generate(size_t n) const;
};

using Weights_generator_ptr = std::shared_ptr<Weights_generator>;

VectorDouble operator * (const VectorDouble &lhs, Double rhs);

template <typename T> T ZeroInitialized(const SymbolsDistributions<T> &d);
template <> Symbol ZeroInitialized<Symbol>(const SymbolsDistributions<Symbol> &);
template <> Double ZeroInitialized<Double>(const SymbolsDistributions<Double> &);
template <> VectorSymbol ZeroInitialized<VectorSymbol>(const SymbolsDistributions<VectorSymbol> &d);
template <> VectorDouble ZeroInitialized<VectorDouble>(const SymbolsDistributions<VectorDouble> &d);

template <typename T>
T mean(const SymbolsDistributions<T> &d, const typename SymbolsDistributions<T>::Factor_type &compressor) {
  auto sum = ZeroInitialized<T>(d);
  Sampler<T> sampler;
  for (auto interval_no : d.get_index()) {
    sum += sampler.InverseTransform(interval_no, d) * d(interval_no, compressor);
  }

  return sum;
}

/**
 * Takes n-th difference of the specified time series.
 *
 * @param x Time series to differentization.
 * @param n Differentize the time series n times.
 *
 * @return Differentized time series.
 */
template <typename Orig_type, typename New_type>
Preprocessed_tseries<Orig_type, New_type> diff_n(Preprocessed_tseries<Orig_type, New_type> x,
                                                 size_t n) {
  Preprocessed_tseries<Orig_type, New_type> diff_x(x.cbegin(), x.cend());
  diff_x.copy_preprocessing_info_from(x);

  for (size_t i = 1; i <= n; ++i) {
    diff_x.push_last_diff_value(diff_x[diff_x.size()-i]);
    for (size_t j = 0; j < diff_x.size()-i; ++j) {
      diff_x[j] = diff_x[j+1] - diff_x[j];
    }
  }
  diff_x.erase(diff_x.end()-n, diff_x.end());

  assert(diff_x.applied_diff_count() == n);

  return diff_x;
}

template <typename T>
void integrate(Forecast<T> &forecast) {
  T last_value;
  while (0 < forecast.applied_diff_count()) {
    last_value = forecast.pop_last_diff_value();
    for (const auto &compressor : forecast.get_index()) {
      forecast(compressor, 0).point += last_value;
      forecast(compressor, 0).left_border += last_value;
      forecast(compressor, 0).right_border += last_value;
      for (size_t j = 1; j < forecast.factors_size(); ++j) {
        forecast(compressor, j).point += forecast(compressor, j - 1).point;
        forecast(compressor, j).left_border +=
            forecast(compressor, j - 1).left_border;
        forecast(compressor, j).right_border +=
            forecast(compressor, j - 1).right_border;
      }
    }
  }
}

template <typename Forward_iterator>
inline void to_code_probabilities(Forward_iterator first, Forward_iterator last) {
  HighPrecDouble base = 2.;
  while (first != last) {
    *first = bignums::pow(base, -(*first));
    ++first;
  }
}

template <typename T>
void form_group_forecasts(ContinuationsDistribution<T> &code_probabilities,
                          const std::vector<Names> &compressors_groups, Weights_generator_ptr weights_generator) {
  for (const auto &group : compressors_groups) {
    if (group.size() > 1) {
      auto group_composite_name = concatenate(group);
      auto weights = weights_generator->generate(group.size());
      for (const auto &continuation : code_probabilities.get_index()) {
        code_probabilities(continuation, group_composite_name) = 0;
        for (size_t i = 0; i < group.size(); ++i) {
          code_probabilities(continuation, group_composite_name) +=
              code_probabilities(continuation, group[i]) * weights[i];
        }
      }
    }
  }
}

template <typename T>
ContinuationsDistribution<T> to_probabilities(ContinuationsDistribution<T> code_probabilities) {
  Double cumulated_sum;
  for (const auto &compressor : code_probabilities.get_factors()) {
    cumulated_sum = .0;
    for (const auto &continuation : code_probabilities.get_index()) {
      cumulated_sum += static_cast<Double>(code_probabilities(continuation, compressor));
    }

    for (const auto &continuation : code_probabilities.get_index()) {
      code_probabilities(continuation, compressor) /= cumulated_sum;
    }
  }

  return code_probabilities;
}

template <typename T>
ContinuationsDistribution<T> merge(const std::vector<ContinuationsDistribution<T>> &tables,
                                   const std::vector<size_t> &alphabets,
                                   const std::vector<Double> &weights) {
  assert(tables.size() == weights.size());
  assert(std::is_sorted(begin(alphabets), end(alphabets)));

  std::vector<size_t> steps(tables.size());
  auto maximal_alphabet = alphabets[alphabets.size() - 1];
  std::transform(begin(alphabets), end(alphabets), begin(steps),
                 [&maximal_alphabet](size_t item) {
                   return maximal_alphabet / item;
                 });

  ContinuationsDistribution<T> result(tables[tables.size() - 1]);
  for (const auto &continuation : result.get_index()) {
    for (const auto &compressor : result.get_factors()) {
      result(continuation, compressor) = .0;
      for (size_t i = 0; i < tables.size(); ++i) {
        result(continuation, compressor) +=
            tables[i](continuation/steps[i], compressor)*weights[i];
      }
    }
  }
  result.copy_preprocessing_info_from(tables.back());

  return result;
}

template <typename T>
Forecast<T> to_pointwise_forecasts(const ContinuationsDistribution<T> &table, size_t h,
                                   double confidence_probability = 0.95) {
  Forecast<T> result;
  for (size_t i = 0; i < h; ++i) {
    SymbolsDistributions<T> d = cumulated_for_step(table, i);
    for (auto compressor : d.get_factors()) {
      result(compressor, i).point = mean(d, compressor);
    }
  }
  result.copy_preprocessing_info_from(table);

  return result;
}

template <typename T>
SymbolsDistributions<T> cumulated_for_step(const ContinuationsDistribution<T> &table,
                                           std::size_t step)  {
  assert(step <= 1000);

  SymbolsDistributions<T> result;
  for (const auto &continuation : table.get_index()) {
    for (const auto &compressor : table.get_factors()) {
      result(continuation[step], compressor) = 0;
    }
  }

  for (const auto &continuation : table.get_index()) {
    for (const auto &compressor : table.get_factors()) {
      result(continuation[step], compressor) += table(continuation, compressor);
    }
  }
  result.copy_preprocessing_info_from(table);

  return result;
}
} // of itp

#endif // ITP_TTRANSFORMATIONS_H_INCLUDED_
