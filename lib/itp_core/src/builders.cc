#include "builders.h"
#include "compression_prediction.h"
#include "itp_exceptions.h"
#include "NonCompressionAlgorithmAdaptor.h"

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

std::vector<itp::VectorDouble> Convert(const std::vector<std::vector<double>> &series) {
  if (series.empty()) {
    return {};
  }

  const size_t number_of_points = series[0].size();
  for (size_t i = 1; i < series.size(); ++i) {
    if (series[i].size() != number_of_points) {
      throw itp::DifferentHistoryLengthsError("The length of series with number " + std::to_string(i)
                                              + " differs from the length of the first series");
    }
  }

  std::vector<itp::VectorDouble> to_return(number_of_points, itp::VectorDouble(series.size()));
  for (size_t series_num = 0; series_num < series.size(); ++series_num) {
    for (size_t point_num = 0; point_num < series[series_num].size(); ++point_num) {
      to_return[point_num][series_num] = series[series_num][point_num];
    }
  }

  return to_return;
}

std::vector<std::vector<double>> Convert(const std::vector<itp::VectorDouble> &res) {
  if (res.empty()) {
    return {};
  }

  const size_t kNumberOfSeries = res[0].size();
  std::vector<std::vector<double>> to_return(kNumberOfSeries, std::vector<double>(res.size()));
  for (size_t point_num = 0; point_num < res.size(); ++point_num) {
    if (res[point_num].size() != kNumberOfSeries) {
      throw itp::DifferentHistoryLengthsError("The number of series for element with number "
                                              + std::to_string(point_num)
                                              + " differs from the number of the first series");}
    
    for (size_t series_num = 0; series_num < kNumberOfSeries; ++series_num) {
      to_return[series_num][point_num] = res[point_num][series_num];
    }
  }
  
  return to_return;
}

std::map<std::string, std::vector<std::vector<double>>> Convert(
		const std::map<std::string, std::vector<itp::VectorDouble>> &res) {
  std::map<std::string, std::vector<std::vector<double>>> to_return;
  for (const auto &pair : res) {
    to_return[pair.first] = Convert(pair.second);
  }

  return to_return;
}
