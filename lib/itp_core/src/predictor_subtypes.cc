#include "predictor.h"
#include "sampler.h"
#include "compressors.h"
//#include "auxilary.h"

#include <cassert>
#include <fstream>
#include <algorithm>
#include <sstream>

namespace itp {

/*Double mean(const SymbolsDistributions &d,
            const SymbolsDistributions::Factor_type &compressor,
            Sampler_ptr sampler, const Desample_info &info) {
  Double sum {0};
  for (auto interval_no : d.get_index()) {
    sum += d(interval_no, compressor) * sampler->desample(interval_no, info);
  }

  return sum;
}*/

/*std::ostream & operator << (std::ostream &ost, const ContinuationsDistribution &table) {
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
}*/

/*Names split_concatenated_names(std::string concatenated_names, char separator) {
  std::istringstream iss(concatenated_names);
  Names result;
  std::string name;
  while (std::getline(iss, name, separator)) {
    result.push_back(name);
  }

  return result;
}

std::vector<Names> split_concatenated_names(const std::vector<std::string>
                                            &concatenated_names,
                                            char separator) {
  std::vector<Names> result;
  for (const auto &names : concatenated_names) {
    result.push_back(split_concatenated_names(names));
  }

  return result;
}

std::string concatenate(const Names &compressors, char separator) {
  if (compressors.size() == 0) {
    return std::string {};
  }
  std::ostringstream oss;
  oss << compressors[0];
  for (size_t i = 1; i < compressors.size(); ++i) {
    oss << separator << compressors[i];
  }

  return oss.str();
}

Names find_all_distinct_names(const std::vector<Names> &compressors) {
  Names list_of_all_names;
  std::for_each(begin(compressors), end(compressors), [&list_of_all_names](Names names) {
      std::copy(std::begin(names), std::end(names),
                std::back_inserter(list_of_all_names));
    });
  std::sort(begin(list_of_all_names), end(list_of_all_names));

  Names unique_names;
  std::unique_copy(begin(list_of_all_names), end(list_of_all_names), std::back_inserter(unique_names));

  return unique_names;
}

std::vector<Double> Weights_generator::generate(size_t n) const {
  assert(0 < n);
  std::vector<Double> result(n);
  std::fill(begin(result), end(result), 1./n);
  return result;
}

std::vector<Double> Countable_weights_generator::generate(size_t n) const {
  assert(0 < n);

  std::vector<Double> result(n);
  for (size_t i = 0; i < n - 1; ++i) {
    result[i] = 1. / (i + 1.) - 1. / (i + 2.);
  }
  result[n - 1] = 1. / n;

  return result;
}*/
} // itp
