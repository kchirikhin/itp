#ifndef ITP_DTYPES_H_INCLUDED_
#define ITP_DTYPES_H_INCLUDED_

#include "continuation.h"
#include "ftable.h"
#include "tseries.h"

namespace itp {

template <typename T>
using ContinuationsDistribution = Table_with_preproc_info<Continuation<Symbol>,
                                                          std::string, HighPrecDouble, T>;

template <typename T>
using Forecast = Table_with_preproc_info<std::string, size_t, Forecast_point<Double>, T>;

template <typename T>
using SymbolsDistributions = Table_with_preproc_info<Symbol, std::string, HighPrecDouble, T>;
} // of itp

#endif // ITP_DTYPES_H_INCLUDED_
