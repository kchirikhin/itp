#include "compression_prediction.h"
#include "compressors.h"
#include "ttransformations.h"

namespace itp {
Multialphabet_distribution_predictor::Multialphabet_distribution_predictor(Codes_lengths_computer_ptr<Double> codes_lengths_computer,
                                                                           Sampler_ptr sampler,
                                                                           size_t max_q,
                                                                           size_t difference_order)
    : Compression_based_predictor<Double, Double> {difference_order},
  codes_lengths_computer{codes_lengths_computer}, sampler{sampler},
  partitions_weights_gen {std::make_shared<Countable_weights_generator>()} {
    assert(codes_lengths_computer != nullptr);
    assert(sampler != nullptr);
    assert(max_q != 0);
    assert(is_power_of_two(max_q));
    log2_max_partition_cardinality = log2(max_q);
  }

ContinuationsDistribution<Double>
Multialphabet_distribution_predictor::obtain_code_probabilities(const Preprocessed_tseries<Double, Double> &history, size_t horizont, const Group &archivers) const {
  size_t N = log2_max_partition_cardinality;
  std::vector<ContinuationsDistribution<Double>> tables(N);
  std::vector<size_t> alphabets(N);
  for (size_t i = 0; i < N; ++i) {
    alphabets[i] = static_cast<size_t>(pow(2, i+1));
    auto sampled_ts = sampler->sample(history, alphabets[i]);
    tables[i] = codes_lengths_computer->append_each_trajectory_and_compute(sampled_ts.to_plain_tseries(),
                                                                           static_cast<size_t>(pow(2, i + 1)),
                                                                           horizont, archivers);
    tables[i].copy_preprocessing_info_from(sampled_ts);
  }

  auto message_length = history.size() + horizont;
  for (size_t i = 0; i < N; ++i) {
    add_value_to_each(begin(tables[i]), end(tables[i]), (N - i - 1) * message_length);
  }
  auto global_minimal_code_length = min_value_of_all_tables<decltype(tables)::const_iterator, Double>(tables.cbegin(), tables.cend());
  for (auto &table : tables) {
    add_value_to_each(begin(table), end(table), -global_minimal_code_length);
    to_code_probabilities(begin(table), end(table));
  }

  auto table = merge(tables, alphabets, partitions_weights_gen->generate(N));
  table.copy_preprocessing_info_from(tables.back());

  return table;
}

Real_distribution_predictor::Real_distribution_predictor(Codes_lengths_computer_ptr<Double> codes_lengths_computer, Sampler_ptr sampler, size_t partition_cardinality, size_t difference_order)
    : Single_alphabet_distribution_predictor {codes_lengths_computer, difference_order}, sampler{sampler},
      partition_cardinality {partition_cardinality} {
  // DO NOTHING
}

Preprocessed_tseries<Double, Symbol>
Real_distribution_predictor::sample(const Preprocessed_tseries<Double, Double> &history) const {
  auto sampling_result = sampler->sample(history, partition_cardinality);
  return sampling_result;
}

Discrete_distribution_predictor::Discrete_distribution_predictor(Codes_lengths_computer_ptr<Symbol> codes_lengths_computer, Sampler_ptr sampler, size_t difference_order)
    : Single_alphabet_distribution_predictor {codes_lengths_computer, difference_order}, sampler{sampler} {
  // DO NOTHING
}

Preprocessed_tseries<Symbol, Symbol>
Discrete_distribution_predictor::sample(const Preprocessed_tseries<Symbol, Symbol> &history) const {
  return sampler->normalize(history);
}
} // itp
