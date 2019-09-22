#ifndef ITP_COMPRESSION_PREDICTION_H_INCLUDED_
#define ITP_COMPRESSION_PREDICTION_H_INCLUDED_

#include "predictor.h"
#include "compressors.h"
#include "compnames.h"

#include <algorithm>

namespace itp {

template <typename T>
class CodeLengthsComputer {
 public:
  using Trajectories = std::vector<Continuation<Symbol>>;

  virtual ~CodeLengthsComputer() = default;

  virtual ContinuationsDistribution<T>
  AppendEachTrajectoryAndCompute(const PlainTimeSeries<Symbol> &history,
                                 size_t alphabet, size_t length_of_continuation,
                                 const Names &compressors_to_compute,
                                 const Trajectories &possible_continuations) const;
  
  virtual ContinuationsDistribution<T>
  AppendEachTrajectoryAndCompute(const PlainTimeSeries<Symbol> &history, size_t alphabet,
                                 size_t length_of_continuation,
                                 const Names &compressors_to_compute) const;

 private:
  static constexpr size_t kBitsInByte = 8;
};

template <typename T>
using CodeLengthsComputerPtr = std::shared_ptr<CodeLengthsComputer<T>>;

template <typename OrigType, typename NewType>
class CompressionBasedPredictor : public Distribution_predictor<OrigType, NewType> {
 public:
  explicit CompressionBasedPredictor(size_t difference_order = 0);
  explicit CompressionBasedPredictor(Weights_generator_ptr weights_generator,
                                     size_t difference_order = 0);

  ContinuationsDistribution<OrigType> Predict(Preprocessed_tseries<OrigType, NewType> history, size_t horizont,
                                              const std::vector<Names> &compressors) const override final;

  void set_difference_order(size_t n);
  size_t get_difference_order() const;

 protected:
  virtual ContinuationsDistribution<OrigType>
  obtain_code_probabilities(const Preprocessed_tseries<OrigType, NewType> &history, size_t horizont,
                            const Names &compressors) const = 0;

 private:
  Weights_generator_ptr weights_generator;
  size_t difference_order;
};

template <typename DoubleT>
class Multialphabet_distribution_predictor : public CompressionBasedPredictor<DoubleT, DoubleT> {
 public:
  Multialphabet_distribution_predictor() = delete;
  Multialphabet_distribution_predictor(CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
                                       SamplerPtr<DoubleT> sampler, size_t max_q, size_t difference = 0);

  ContinuationsDistribution<DoubleT>
  obtain_code_probabilities(const Preprocessed_tseries<DoubleT, DoubleT> &ts, size_t horizont,
                            const Group &compressors) const override;
  
 private:
  CodeLengthsComputerPtr<DoubleT> codes_lengths_computer;
  SamplerPtr<DoubleT> sampler;
  size_t log2_max_partition_cardinality;
  Weights_generator_ptr partitions_weights_gen;
};

template <typename OrigType, typename NewType>
class Single_alphabet_distribution_predictor : public CompressionBasedPredictor<OrigType, NewType> {
 public:
  Single_alphabet_distribution_predictor() = delete;
  explicit Single_alphabet_distribution_predictor(CodeLengthsComputerPtr<OrigType>, size_t = 0);
 protected:
  ContinuationsDistribution<OrigType>
  obtain_code_probabilities(const Preprocessed_tseries<OrigType, NewType> &, size_t,
                            const Names &) const override final;
  virtual Preprocessed_tseries<OrigType, Symbol>
  sample(const Preprocessed_tseries<OrigType,NewType> &) const = 0;

 private:
  CodeLengthsComputerPtr<OrigType> codes_lengths_computer;
};

template <typename DoubleT>
class Real_distribution_predictor : public Single_alphabet_distribution_predictor<DoubleT, DoubleT> {
 public:
  Real_distribution_predictor() = delete;
  Real_distribution_predictor(CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
                              SamplerPtr<DoubleT> sampler, size_t partition_cardinality,
                              size_t difference = 0);
 protected:
  Preprocessed_tseries<DoubleT, Symbol>
  sample(const Preprocessed_tseries<DoubleT, DoubleT> &history) const override;
  
 private:
  SamplerPtr<DoubleT> sampler;
  size_t partition_cardinality;
};

template <typename DoubleT, typename SymbolT>
class Discrete_distribution_predictor : public Single_alphabet_distribution_predictor<DoubleT, SymbolT> {
 public:
  Discrete_distribution_predictor() = delete;
  Discrete_distribution_predictor(CodeLengthsComputerPtr<DoubleT> codes_lengths_computer,
                                  SamplerPtr<SymbolT> sampler, size_t difference = 0);

 protected:
  Preprocessed_tseries<DoubleT, Symbol> sample(const Preprocessed_tseries<DoubleT, SymbolT> &history) const override;

 private:
  SamplerPtr<SymbolT> sampler;
};

template <typename T>
ContinuationsDistribution<T>
CodeLengthsComputer<T>::AppendEachTrajectoryAndCompute(const PlainTimeSeries<Symbol> &history,
                                                       size_t alphabet, size_t length_of_continuation,
                                                       const Names &compressors_to_compute,
                                                       const Trajectories &possible_continuations) const {
  assert(length_of_continuation <= 100);
  assert(alphabet > 0);

  Compressors_pool::get_instance().init_compressors_for_ts(0, alphabet - 1, length_of_continuation);

  ContinuationsDistribution<T> result(std::begin(possible_continuations), std::end(possible_continuations),
                                      std::begin(compressors_to_compute), std::end(compressors_to_compute));
  size_t full_series_length = history.size() + length_of_continuation;
  std::unique_ptr<Symbol[]> buffer(new Symbol[full_series_length]);
  std::copy(history.cbegin(), history.cend(), buffer.get());
  for (const auto &continuation : possible_continuations) {
    std::copy(continuation.cbegin(), continuation.cend(), buffer.get() + history.size());

    for (size_t j = 0; j < result.factors_size(); ++j) {
      result(continuation, compressors_to_compute[j]) =
          Compressors_pool::get_instance()(compressors_to_compute[j], buffer.get(),
                                           history.size() + length_of_continuation) * kBitsInByte;
    }
  }

  std::cout << result;

  return result;
}

template <typename T>
ContinuationsDistribution<T>
CodeLengthsComputer<T>::AppendEachTrajectoryAndCompute(const PlainTimeSeries<Symbol> &history,
                                                       size_t alphabet, size_t length_of_continuation,
                                                       const Names &compressors_to_compute) const {
  assert(0 < alphabet);
  std::vector<Continuation<Symbol>> possible_continuations;
  Continuation<Symbol> continuation(alphabet, length_of_continuation);
  for (size_t i = 0; i < pow(alphabet, length_of_continuation); ++i) {
    possible_continuations.push_back(continuation++);
  }

  return AppendEachTrajectoryAndCompute(history, alphabet, length_of_continuation,
                                            compressors_to_compute, possible_continuations);
}

template <typename OrigType, typename NewType>
ContinuationsDistribution<OrigType>
CompressionBasedPredictor<OrigType, NewType>::Predict(Preprocessed_tseries<OrigType, NewType> history,
                                                      size_t horizont,
                                                      const std::vector<Names> &compressors) const  {
  auto differentized_history = diff_n(history, difference_order);
  auto distinct_single_compressors = find_all_distinct_names(compressors);
  auto code_probabilities_result =
      obtain_code_probabilities(differentized_history, horizont,
                                distinct_single_compressors);
  form_group_forecasts(code_probabilities_result, compressors, weights_generator);
  return to_probabilities(code_probabilities_result);
}

template <typename DoubleT>
Multialphabet_distribution_predictor<DoubleT>::Multialphabet_distribution_predictor(CodeLengthsComputerPtr<DoubleT> codes_lengths_computer, SamplerPtr<DoubleT> sampler, size_t max_q, size_t difference_order)
    : CompressionBasedPredictor<DoubleT, DoubleT> {difference_order},
  codes_lengths_computer { codes_lengths_computer }, sampler { sampler },
  partitions_weights_gen { std::make_shared<Countable_weights_generator>() } {
    assert(codes_lengths_computer != nullptr);
    assert(sampler != nullptr);
    assert(max_q != 0);
    assert(IsPowerOfTwo(max_q));
    log2_max_partition_cardinality = log2(max_q);
  }

template <typename DoubleT>
ContinuationsDistribution<DoubleT>
Multialphabet_distribution_predictor<DoubleT>::obtain_code_probabilities(const Preprocessed_tseries<DoubleT, DoubleT> &history, size_t horizont, const Group &archivers) const {
  size_t N = log2_max_partition_cardinality;
  std::vector<ContinuationsDistribution<DoubleT>> tables(N);
  std::vector<size_t> alphabets(N);
  for (size_t i = 0; i < N; ++i) {
    auto sampled_ts = sampler->Transform(history, static_cast<size_t>(pow(2, i+1)));

    // In the vector case it will differ from 2^(i+1)!
    alphabets[i] = sampled_ts.get_sampling_alphabet();
    tables[i] = codes_lengths_computer->AppendEachTrajectoryAndCompute(sampled_ts.to_plain_tseries(),
                                                                       sampled_ts.get_sampling_alphabet(),
                                                                       horizont, archivers);
    tables[i].copy_preprocessing_info_from(sampled_ts);
  }

  auto message_length = history.size() + horizont;
  for (size_t i = 0; i < N; ++i) {
    add_value_to_each(begin(tables[i]), end(tables[i]), (N - i - 1) * message_length);
  }
  auto global_minimal_code_length = min_value_of_all_tables<typename decltype(tables)::const_iterator, HighPrecDouble>(tables.cbegin(), tables.cend());
  for (auto &table : tables) {
    add_value_to_each(begin(table), end(table), -global_minimal_code_length);
    to_code_probabilities(begin(table), end(table));
  }

  auto table = merge(tables, alphabets, partitions_weights_gen->generate(N));
  table.copy_preprocessing_info_from(tables.back());

  return table;
}

template <typename DoubleT>
Real_distribution_predictor<DoubleT>::Real_distribution_predictor(CodeLengthsComputerPtr<DoubleT> codes_lengths_computer, SamplerPtr<DoubleT> sampler, size_t partition_cardinality, size_t difference_order)
    : Single_alphabet_distribution_predictor<DoubleT, DoubleT> {codes_lengths_computer, difference_order}, sampler { sampler }, partition_cardinality { partition_cardinality } {
  // DO NOTHING
}

template <typename DoubleT>
Preprocessed_tseries<DoubleT, itp::Symbol>
Real_distribution_predictor<DoubleT>::sample(const Preprocessed_tseries<DoubleT, DoubleT> &history) const {
  auto sampling_result = sampler->Transform(history, partition_cardinality);
  return sampling_result;
}

template <typename DoubleT, typename SymbolT>
Discrete_distribution_predictor<DoubleT, SymbolT>::Discrete_distribution_predictor(CodeLengthsComputerPtr<DoubleT> codes_lengths_computer, SamplerPtr<SymbolT> sampler, size_t difference_order)
    : Single_alphabet_distribution_predictor<DoubleT, SymbolT> { codes_lengths_computer, difference_order }, sampler { sampler } {
  // DO NOTHING
}

template <typename DoubleT, typename SymbolT>
Preprocessed_tseries<DoubleT, Symbol>
Discrete_distribution_predictor<DoubleT, SymbolT>::sample(const Preprocessed_tseries<DoubleT, SymbolT> &history) const {
  return sampler->Transform(history);
}

template <typename OrigType, typename NewType>
ContinuationsDistribution<OrigType>
Single_alphabet_distribution_predictor<OrigType, NewType>::obtain_code_probabilities(const Preprocessed_tseries<OrigType, NewType> &history, size_t horizont, const Names &compressors) const {
  auto sampled_tseries = sample(history);
  auto table = codes_lengths_computer->AppendEachTrajectoryAndCompute(sampled_tseries.to_plain_tseries(),
                                                                      sampled_tseries.get_sampling_alphabet(),
                                                                      horizont, compressors);
  auto min = *std::min_element(begin(table), end(table));
  add_value_to_each(begin(table), end(table), -min);
  to_code_probabilities(begin(table), end(table));
  table.copy_preprocessing_info_from(sampled_tseries);
  
  return table;
}

template <typename OrigType, typename NewType>
CompressionBasedPredictor<OrigType, NewType>::CompressionBasedPredictor(size_t difference_order)
    : CompressionBasedPredictor<OrigType, NewType> { std::make_shared<Weights_generator>(), difference_order } {
  // DO NOTHING
}

template <typename OrigType, typename NewType>
CompressionBasedPredictor<OrigType, NewType>::CompressionBasedPredictor(Weights_generator_ptr weights_generator,
                                                                        size_t difference_order)
    : weights_generator {weights_generator}, difference_order {difference_order} {
  // DO NOTHING
}

template <typename OrigType, typename NewType>
void CompressionBasedPredictor<OrigType, NewType>::set_difference_order(size_t n) {
  difference_order = n;
}

template <typename OrigType, typename NewType>
size_t CompressionBasedPredictor<OrigType, NewType>::get_difference_order() const {
  return difference_order;
}

template <typename OrigType, typename NewType>
Single_alphabet_distribution_predictor<OrigType, NewType>::Single_alphabet_distribution_predictor(CodeLengthsComputerPtr<OrigType> codes_lengths_computer, size_t)
    : codes_lengths_computer {codes_lengths_computer} {
  // DO NOTHING
}
} // of itp

#endif // ITP_COMPRESSION_PREDICTION_H_INCLUDED_
