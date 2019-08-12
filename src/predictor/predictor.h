#ifndef PREDICTOR_PREDICTOR_H
#define PREDICTOR_PREDICTOR_H

#include "sampler.h"
#include "dtypes.h"
#include "ttransformations.h"

#include <iterator>
#include <iostream>
#include <memory>
#include <cmath>

// Interface of the library.
namespace itp {
    //std::pair<Symbol, Symbol> find_confidence_interval(const SymbolsDistributions &, double);


    template <typename T>
    std::ostream &operator << (std::ostream &ost, const ContinuationsDistribution<T> &table);

    inline bool is_power_of_two(std::size_t n) {
        return (n > 0 && ((n & (n - 1)) == 0));
    }
  
    template <typename Orig_type, typename New_type>
    class Distribution_predictor {
        // static_assert(std::is_arithmetic<New_type>::value, "New_type should be an arithmetic type");
    public:
        virtual ~Distribution_predictor() = default;

        virtual ContinuationsDistribution<Orig_type> predict(Preprocessed_tseries<Orig_type, New_type> history, size_t horizont,
                const std::vector<Names> &compressors) const = 0;
    };

    template <typename Orig_type, typename New_type>
    using Distribution_predictor_ptr = std::shared_ptr<Distribution_predictor<Orig_type, New_type>>;

    template <typename Orig_type, typename New_type>
    class Pointwise_predictor {
        // static_assert(std::is_arithmetic<New_type>::value, "New_type should be an arithmetic type");
    public:
        virtual ~Pointwise_predictor() = default;

        virtual Forecast<Orig_type> predict(Preprocessed_tseries<Orig_type, New_type> history, size_t horizont,
                                 const std::vector<Names> &compressors) const = 0;
    };

    template <typename Orig_type, typename New_type>
    using Pointwise_predictor_ptr = std::shared_ptr<Pointwise_predictor<Orig_type, New_type>>;

    template <typename Orig_type, typename New_type>
    class Basic_pointwise_predictor : public Pointwise_predictor<Orig_type, New_type> {
    public:
        Basic_pointwise_predictor(Distribution_predictor_ptr<Orig_type, New_type> distribution_predictor);
        Forecast<Orig_type> predict(Preprocessed_tseries<Orig_type, New_type> history, size_t horizont,
                         const std::vector<Names> &compressors) const override;
    private:
        Distribution_predictor_ptr<Orig_type, New_type> distribution_predictor;
    };

    /**
     * Decorator pattern implementation.
     *
     */
    template <typename Orig_type, typename New_type>
    class Sparse_predictor : public Pointwise_predictor<Orig_type, New_type> {
    public:
        Sparse_predictor(Pointwise_predictor_ptr<Orig_type, New_type> pointwise_predictor, size_t sparse);

        Forecast<Orig_type> predict(Preprocessed_tseries<Orig_type, New_type> history, size_t horizont,
                         const std::vector<Names> &compressors) const override final;
    private:
        Pointwise_predictor_ptr<Orig_type, New_type> pointwise_predictor;
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

template <typename Orig_type, typename New_type>
itp::Forecast<Orig_type> itp::Sparse_predictor<Orig_type, New_type>::predict(Preprocessed_tseries<Orig_type, New_type> history, size_t horizont, const std::vector<Names> &compressors) const {
    std::vector<Forecast<Orig_type>> results(sparse);
    size_t sparsed_horizont = ceil(horizont / static_cast<double>(sparse));
    for (size_t i = 0; i < sparse; ++i) {
        PlainTimeSeries<New_type> sparse_ts_data;
        for (size_t j = i; j < history.size(); j += sparse) {
            sparse_ts_data.push_back(history[j]);
        }
        results[i] = pointwise_predictor->predict(sparse_ts_data, sparsed_horizont, compressors);
    }

    Forecast<Orig_type> result;
    Forecast<Orig_type> full_first_steps =
        pointwise_predictor->predict(history, sparsed_horizont, compressors);
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

template <typename Orig_type, typename New_type>
itp::Basic_pointwise_predictor<Orig_type, New_type>::Basic_pointwise_predictor(Distribution_predictor_ptr<Orig_type, New_type> distribution_predictor)
    : distribution_predictor {distribution_predictor} {}

template <typename Orig_type, typename New_type>
itp::Forecast<Orig_type> itp::Basic_pointwise_predictor<Orig_type, New_type>::predict(Preprocessed_tseries<Orig_type, New_type> ts, size_t horizont,
                                                                            const std::vector<Names> &compressors) const {
    auto distribution = distribution_predictor->predict(ts, horizont, compressors);
    auto forecasts = to_pointwise_forecasts(distribution, horizont);
    integrate(forecasts);
    return forecasts;
}

template <typename Orig_type, typename New_type>
itp::Sparse_predictor<Orig_type, New_type>::Sparse_predictor(Pointwise_predictor_ptr<Orig_type, New_type> pointwise_predictor, size_t sparse)
    : pointwise_predictor {pointwise_predictor}, sparse {sparse} {
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

#endif //PREDICTOR_PREDICTOR_H
