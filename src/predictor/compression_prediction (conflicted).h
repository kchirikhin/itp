#ifndef COMPRESSION_PREDICTION_H
#define COMPRESSION_PREDICTION_H

#include "predictor.h"
#include "compressors.h"
#include "compnames.h"

#include <algorithm>

namespace itp {
    template <typename T>
    class Codes_lengths_computer {
    public:
        using Trajectories = std::vector<Continuation<Symbol_t>>;

        virtual ~Codes_lengths_computer() = default;

        virtual Continuations_distribution<T>
        append_each_trajectory_and_compute(const Plain_tseries<Symbol_t> &history,
                                           size_t alphabet, size_t length_of_continuation,
                                           const Names &compressors_to_compute,
                                           const Trajectories &possible_continuations) const;
        virtual Continuations_distribution<T>
        append_each_trajectory_and_compute(const Plain_tseries<Symbol_t> &history, size_t alphabet,
                                           size_t length_of_continuation,
                                           const Names &compressors_to_compute) const;
    private:
        static constexpr size_t BITS_IN_BYTE = 8;
    };


    template <typename T>
    using Codes_lengths_computer_ptr = std::shared_ptr<Codes_lengths_computer<T>>;

    template <typename Orig_type, typename New_type>
    class Compression_based_predictor : public Distribution_predictor<Orig_type, New_type> {
    public:
        explicit Compression_based_predictor(size_t difference_order=0);
        explicit Compression_based_predictor(Weights_generator_ptr weights_generator,
                                             size_t difference_order = 0);

        Continuations_distribution<Orig_type> predict(Preprocessed_tseries<Orig_type, New_type> history, size_t horizont,
                                              const std::vector<Names> &compressors) const override final;

        void set_difference_order(size_t n);
        size_t get_difference_order() const;

    protected:
        virtual Continuations_distribution<New_type> obtain_code_probabilities(const Preprocessed_tseries<Orig_type, New_type> &history,
                                                                                size_t horizont,
                                                                                const Names &compressors) const = 0;
    private:
        Weights_generator_ptr weights_generator;
        size_t difference_order;
    };

    class Multialphabet_distribution_predictor : public Compression_based_predictor<Double_t, Prec_double_t> {
    public:
        Multialphabet_distribution_predictor() = delete;
        Multialphabet_distribution_predictor(Codes_lengths_computer_ptr<Prec_double_t> codes_lengths_computer,
                                             Sampler_ptr sampler, size_t max_q, size_t difference=0);

        Continuations_distribution<Prec_double_t>
        obtain_code_probabilities(const Preprocessed_tseries<Double_t, Prec_ouble_t> &ts, size_t horizont,
                                  const Group &compressors) const override;
    private:
        Codes_lengths_computer_ptr<Prec_double_t> codes_lengths_computer;
        Sampler_ptr sampler;
        size_t log2_max_partition_cardinality;
        Weights_generator_ptr partitions_weights_gen;
    };

    template <typename Orig_type, typename New_type>
    class Single_alphabet_distribution_predictor : public Compression_based_predictor<Orig_type, New_type> {
    public:
        Single_alphabet_distribution_predictor() = delete;
        explicit Single_alphabet_distribution_predictor(Codes_lengths_computer_ptr<Orig_type>, size_t=0);
    protected:
        Continuations_distribution<Orig_type> obtain_code_probabilities(const Preprocessed_tseries<Orig_type, New_type> &, size_t,
                                                                const Names &) const override final;
        virtual Preprocessed_tseries<Orig_type, Symbol_t> sample(const Preprocessed_tseries<Orig_type,New_type> &) const = 0;

    private:
        Codes_lengths_computer_ptr<Orig_type> codes_lengths_computer;
    };

    class Real_distribution_predictor : public Single_alphabet_distribution_predictor<Double_t, Double_t> {
    public:
        Real_distribution_predictor() = delete;
        Real_distribution_predictor(Codes_lengths_computer_ptr<Double_t> codes_lengths_computer,
                                    Sampler_ptr sampler, size_t partition_cardinality,
                                    size_t difference=0);
    protected:
        Preprocessed_tseries<Double_t, Symbol_t> sample(const Preprocessed_tseries<Double_t, Double_t> &history) const override;
    private:
        Sampler_ptr sampler;
        size_t partition_cardinality;
    };

    class Discrete_distribution_predictor : public Single_alphabet_distribution_predictor<Symbol_t, Symbol_t> {
    public:
        Discrete_distribution_predictor() = delete;
        Discrete_distribution_predictor(Codes_lengths_computer_ptr<Symbol_t> codes_lengths_computer,
                                        Sampler_ptr sampler, size_t difference=0);
    protected:
        Preprocessed_tseries<Symbol_t, Symbol_t> sample(const Preprocessed_tseries<Symbol_t, Symbol_t> &history) const override;
    private:
        Sampler_ptr sampler;
    };

    template <typename Orig_type, typename New_type>
    using Distribution_predictor_ptr = std::shared_ptr<Distribution_predictor<Orig_type, New_type>>;
    using Discrete_distribution_predictor_ptr = std::shared_ptr<Discrete_distribution_predictor>;
    using Real_distribution_predictor_ptr = std::shared_ptr<Real_distribution_predictor>;
} // of itp

template <typename T>
itp::Continuations_distribution<T>
itp::Codes_lengths_computer<T>::append_each_trajectory_and_compute(const Plain_tseries<Symbol_t> &history,
                                                                size_t alphabet,
                                                                size_t length_of_continuation,
                                                                const Names &compressors_to_compute,
                                                                const Trajectories &possible_continuations) const {
    assert(length_of_continuation <= 100);
    assert(alphabet > 0);

    Compressors_pool::get_instance().init_compressors_for_ts(0, alphabet-1, length_of_continuation);

    Continuations_distribution<T> result(std::begin(possible_continuations), std::end(possible_continuations),
                                         std::begin(compressors_to_compute), std::end(compressors_to_compute));
    size_t full_series_length = history.size() + length_of_continuation;
    std::unique_ptr<Symbol_t[]> buffer(new Symbol_t[full_series_length]);
    std::copy(history.cbegin(), history.cend(), buffer.get());
    for (const auto &continuation : possible_continuations) {
        std::copy(continuation.cbegin(), continuation.cend(), buffer.get()+history.size());

        for (size_t j = 0; j < result.factors_size(); ++j) {
            result(continuation, compressors_to_compute[j]) =
                Compressors_pool::get_instance()(compressors_to_compute[j], buffer.get(),
                                                 history.size()+length_of_continuation)*BITS_IN_BYTE;
        }
    }

    return result;
}

template <typename T>
itp::Continuations_distribution<T>
itp::Codes_lengths_computer<T>::append_each_trajectory_and_compute(const Plain_tseries<Symbol_t> &history,
                                                                size_t alphabet,
                                                                size_t length_of_continuation,
                                                                const Names &compressors_to_compute) const {
    assert(0 < alphabet);
    std::vector<Continuation<Symbol_t>> possible_continuations;
    Continuation<Symbol_t> continuation(alphabet, length_of_continuation);
    for (size_t i = 0; i < pow(alphabet, length_of_continuation); ++i) {
        possible_continuations.push_back(continuation++);
    }

    return append_each_trajectory_and_compute(history, alphabet, length_of_continuation,
                                              compressors_to_compute, possible_continuations);
}

template <typename Orig_type, typename New_type>
itp::Continuations_distribution<Orig_type>
itp::Compression_based_predictor<Orig_type, New_type>::predict(Preprocessed_tseries<Orig_type, New_type> history,
                                             size_t horizont,
                                             const std::vector<Names> &compressors) const  {
    auto differentized_history = diff_n(history, difference_order);
    auto distinct_single_compressors = find_all_distinct_names(compressors);
    auto code_probabilities_result =
        obtain_code_probabilities(differentized_history, horizont,
                                  distinct_single_compressors);
    form_group_forecasts(code_probabilities_result, compressors, weights_generator);
    return to_probabilities<Orig_type, New_type>(code_probabilities_result);
}

template <typename Orig_type, typename New_type>
itp::Continuations_distribution<Orig_type>
itp::Single_alphabet_distribution_predictor<Orig_type, New_type>::obtain_code_probabilities(const Preprocessed_tseries<Orig_type, New_type> &history,
                                                                          size_t horizont,
                                                                          const Names &compressors) const {
    auto sampled_tseries = sample(history);
    auto table = codes_lengths_computer->append_each_trajectory_and_compute(sampled_tseries.to_plain_tseries(),
                                                                            sampled_tseries.get_sampling_alphabet(), horizont, compressors);
    auto min = *std::min_element(begin(table), end(table));
    // std::cout << table << std::endl;
    add_value_to_each(begin(table), end(table), -min);
    to_code_probabilities(begin(table), end(table));
    // std::cout << table << std::endl;
    table.copy_preprocessing_info_from(sampled_tseries);
    return table;
}

    template <typename Orig_type, typename New_type>
    itp::Compression_based_predictor<Orig_type, New_type>::Compression_based_predictor(size_t difference_order)
        : Compression_based_predictor<Orig_type, New_type> {std::make_shared<Weights_generator>(), difference_order} {}

    template <typename Orig_type, typename New_type>
    itp::Compression_based_predictor<Orig_type, New_type>::Compression_based_predictor(Weights_generator_ptr weights_generator,
                                                                size_t difference_order)
        : weights_generator {weights_generator}, difference_order {difference_order} {}

    template <typename Orig_type, typename New_type>
    void itp::Compression_based_predictor<Orig_type, New_type>::set_difference_order(size_t n) {
        difference_order = n;
    }

    template <typename Orig_type, typename New_type>
    size_t itp::Compression_based_predictor<Orig_type, New_type>::get_difference_order() const {
        return difference_order;
    }

    template <typename Orig_type, typename New_type>
    itp::Single_alphabet_distribution_predictor<Orig_type, New_type>::Single_alphabet_distribution_predictor(Codes_lengths_computer_ptr<Orig_type> codes_lengths_computer,
                                                                                      size_t difference)
        : codes_lengths_computer {codes_lengths_computer} {}

#endif // COMPRESSION_PREDICTION_H
