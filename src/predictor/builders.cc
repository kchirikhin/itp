#include "builders.h"
#include "compression_prediction.h"

std::string trim_line(const std::string &line, const std::function<int(int)> &is_empty_char) {
  if (line.empty()) {
    return line;
  }

  size_t first_non_empty = 0;
  while (is_empty_char(line[first_non_empty])) {
    ++first_non_empty;
    if (first_non_empty == line.size()) {
      return std::string();
    }
  }

  size_t last_non_empty = line.size();
  while (is_empty_char(line[--last_non_empty]));

  return line.substr(first_non_empty, last_non_empty - first_non_empty + 1);
}

void check_args(size_t horizont, size_t difference, int sparse) {
  if (50 < horizont) {
    throw std::invalid_argument("Forecasting horizont is too long (> 50)");
  }

  if (10 < difference) {
    throw std::invalid_argument("Difference order is too big (> 10)");
  }

  if (20 < sparse) {
    throw std::invalid_argument("Sparse value is too big (> 20)");
  }
}

inline void check_quants_count_range(size_t quants_count) {
  if ((0 == quants_count) || (256 < quants_count)) {
    throw std::invalid_argument("Quants count should be greater than zero and not greater than 256");
  }
}

std::map<std::string, std::vector<itp::Double>>
make_forecast_real(const std::vector<itp::Double> &time_series, const itp::Names &compressors_groups,
                   size_t horizont, size_t difference, size_t quants_count, int sparse) {
  check_args(horizont, difference, sparse);
  check_quants_count_range(quants_count);
  
  Forecasting_algorithm_real<itp::Double> make_forecast;
  make_forecast.set_quants_count(quants_count);
  return make_forecast(time_series, compressors_groups, horizont, difference, sparse);
}

std::map<std::string, std::vector<itp::Double>>
make_forecast_multialphabet(const std::vector<double> &history,
                            const itp::Names &compressors_groups, size_t horizont,
                            size_t difference, size_t max_quants_count, int sparse) {
  check_args(horizont, difference, sparse);
  check_quants_count_range(max_quants_count);
  if (!itp::IsPowerOfTwo(max_quants_count)) {
    throw std::invalid_argument("Max quants count should be greater a power of two.");
  }
  
  Forecasting_algorithm_multialphabet<itp::Double> make_forecast;
  make_forecast.set_quants_count(max_quants_count);

  std::vector<itp::Double> transformed_history;
  std::copy(begin(history), end(history), std::back_inserter(transformed_history));
  return make_forecast(transformed_history, compressors_groups, horizont, difference, sparse);
}

std::map<std::string, std::vector<itp::VectorDouble>>
make_forecast_multialphabet_vec(const std::vector<itp::VectorDouble> &history,
                                const itp::Names &compressors_groups, size_t horizont,
                                size_t difference, size_t max_quants_count, int sparse) {
  check_args(horizont, difference, sparse);
  check_quants_count_range(max_quants_count);
  if (!itp::IsPowerOfTwo(max_quants_count)) {
    throw std::invalid_argument("Max quants count should be greater a power of two.");
  }
  
  Forecasting_algorithm_multialphabet<itp::VectorDouble> make_forecast;
  make_forecast.set_quants_count(max_quants_count);

  std::vector<itp::VectorDouble> transformed_history;
  std::copy(begin(history), end(history), std::back_inserter(transformed_history));
  return make_forecast(transformed_history, compressors_groups, horizont, difference, sparse);
}

std::map<std::string, std::vector<itp::Double>>
make_forecast_discrete(const std::vector<itp::Symbol> &history, const std::vector<std::string> &compressors_groups,
                       size_t horizont, size_t difference, int sparse) {
  check_args(horizont, difference, sparse);
  
  Forecasting_algorithm_discrete<itp::Double, itp::Symbol> make_forecast;
  auto res = make_forecast(history, compressors_groups, horizont, difference, sparse);
  return res;
}

std::map<std::string, std::vector<itp::VectorDouble>>
make_forecast_discrete_vec(const std::vector<itp::VectorSymbol> &history,
                           const std::vector<std::string> &compressors_groups, size_t horizont,
                           size_t difference, int sparse) {
  check_args(horizont, difference, sparse);

  Forecasting_algorithm_discrete<itp::VectorDouble, itp::VectorSymbol> make_forecast;
  auto res = make_forecast(history, compressors_groups, horizont, difference, sparse);

  return res;
}
