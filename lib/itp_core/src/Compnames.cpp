#include "Compnames.h"

#include <cassert>
#include <fstream>
#include <algorithm>
#include <sstream>

namespace itp {

Names split_concatenated_names(std::string concatenated_names, char separator) {
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
} // itp
