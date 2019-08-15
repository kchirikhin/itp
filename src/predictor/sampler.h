/**
 * Functions for sampling and desampling real time series.
 */

#ifndef ITP_SAMPLER_H_INCLUDED_
#define ITP_SAMPLER_H_INCLUDED_

#include "dtypes.h"

#include <vector>
#include <utility>
#include <cassert>
#include <memory>

namespace itp {

class Sampler {
 public:
  Sampler() = default;
  explicit Sampler(double);

  void set_indent(double);
  double get_indent() const;

  virtual Sampler* clone();

  Preprocessed_tseries<Double, Symbol> sample(const Preprocessed_tseries<Double, Double> &, size_t);
  Preprocessed_tseries<Symbol, Symbol> normalize(const Preprocessed_tseries<Symbol, Symbol> &);

  template <typename T>
  Double desample(Symbol, const Preproc_info<T> &);

 private:
  double indent = 0.1;
};
using Sampler_ptr = std::shared_ptr<Sampler>;

namespace exp {

template <typename OriginalType>
class Sampler;

template <>
class Sampler<Double> {
 public:
  Preprocessed_tseries<Double, Symbol> Transform(const Preprocessed_tseries<Double, Double> &, size_t);
  Double InverseTransform(Symbol, const Preproc_info<Double> &);

 private:
  double indent_ = 0.1;
};

template <>
class Sampler<Symbol> {
 public:
  Preprocessed_tseries<Symbol, Symbol> Transform(const Preprocessed_tseries<Symbol, Symbol> &);
  Symbol InverseTransform(Symbol, const Preproc_info<Symbol> &);
};
} // exp
} // of itp

template <typename T>
itp::Double itp::Sampler::desample(Symbol s, const Preproc_info<T> &info) {
  if (!info.is_sampled()) {
    return s;
  }
        
  assert(!info.get_desample_table().empty());
  assert(s < info.get_desample_table().size());

  return (info.get_desample_table())[s];
}

#endif // ITP_SAMPLER_H_INCLUDED_
