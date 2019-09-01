#ifndef ITP_PREPROC_INFO_H_INCLUDED_
#define ITP_PREPROC_INFO_H_INCLUDED_

#include "primitive_dtypes.h"

#include <limits>
#include <cassert>
#include <stack>

namespace itp {

/**
 * Class contains all information about preliminary actions with a time series. This information
 * allows to apply inverse transformations.
 */
template <typename T>
class Preproc_info {
 public:
  size_t applied_diff_count() const;
  void push_last_diff_value(const T &);
  T pop_last_diff_value();

  void set_sampling_alphabet(size_t);
  size_t get_sampling_alphabet() const;

  void set_desample_table(const std::vector<T> &);
  const std::vector<T> & get_desample_table() const;
  void clear_dsample_table();
  bool is_sampled() const;

  void set_desample_indent(Double);
  Double get_desample_indent() const;

  void copy_preprocessing_info_from(const Preproc_info<T> &);
   
 private:
  std::stack<T> last_values;
        
  size_t alphabet;

  std::vector<T> desample_table;
  Double desample_indent;
  bool sampled = false;
};
} // of itp

template <typename T>
size_t itp::Preproc_info<T>::applied_diff_count() const {
  return last_values.size();
}

template <typename T>
void itp::Preproc_info<T>::push_last_diff_value(const T &value) {
  last_values.push(value);
}

template <typename T>
T itp::Preproc_info<T>::pop_last_diff_value() {
  T res;
  if (0 < last_values.size()) {
    res = last_values.top();
    last_values.pop();
  }

  return res;
}

template <typename T>
void itp::Preproc_info<T>::set_sampling_alphabet(size_t new_alphabet) {
  alphabet = new_alphabet;
}

template <typename T>
size_t itp::Preproc_info<T>::get_sampling_alphabet() const {
  return alphabet;
}

template <typename T>
void itp::Preproc_info<T>::set_desample_table(const std::vector<T> &new_table) {
  desample_table = new_table;
  sampled = true;
}

template <typename T>
const std::vector<T> & itp::Preproc_info<T>::get_desample_table() const {
  return desample_table;
}

template <typename T>
void itp::Preproc_info<T>::clear_dsample_table() {
  desample_table.clear();
  sampled = false;
}

template <typename T>
bool itp::Preproc_info<T>::is_sampled() const {
  return sampled;
}

template <typename T>
void itp::Preproc_info<T>::set_desample_indent(Double new_indent) {
  desample_indent = new_indent;
}

template <typename T>
itp::Double itp::Preproc_info<T>::get_desample_indent() const {
  return desample_indent;
}

template <typename T>
void itp::Preproc_info<T>::copy_preprocessing_info_from(const Preproc_info<T> &src) {
  last_values = src.last_values;
  alphabet = src.alphabet;
  desample_table = src.desample_table;  
  desample_indent = src.desample_indent;
  sampled = src.sampled;
}

#endif // of ITP_PREPROC_INFO_H_INCLUDED_
