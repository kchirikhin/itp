#include "ttransformations.h"

namespace itp {

VectorDouble operator * (const VectorDouble &lhs, Double rhs) {
  VectorDouble to_return = lhs;
  for (auto &value : to_return) {
    value *= rhs;
  }

  return to_return;
}

std::vector<Double> Weights_generator::generate(size_t n) const {
  return std::vector<Double>(n, 1. / static_cast<Double>(n));
}

std::vector<Double> Countable_weights_generator::generate(size_t n) const {
  assert(0 < n);

  std::vector<Double> result(n);
  for (size_t i = 0; i < n - 1; ++i) {
    result[i] = 1. / (i + 1.) - 1. / (i + 2.);
  }
  result[n - 1] = 1. / n;

  return result;
}

template <>
Symbol ZeroInitialized<Symbol>(const SymbolsDistributions<Symbol> &) {
  return 0;
}

template <>
Double ZeroInitialized<Double>(const SymbolsDistributions<Double> &) {
  return 0.;
}

template <>
VectorSymbol ZeroInitialized<VectorSymbol>(const SymbolsDistributions<VectorSymbol> &d) {
  const auto kCountOfSeries = d.get_desample_table().size();
  return VectorSymbol(kCountOfSeries);
}

template <>
VectorDouble ZeroInitialized<VectorDouble>(const SymbolsDistributions<VectorDouble> &d) {
  const auto kCountOfSeries = d.get_desample_table().size();
  return VectorDouble(kCountOfSeries);
}
} // itp
