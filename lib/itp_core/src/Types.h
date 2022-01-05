#ifndef ITP_DTYPES_H_INCLUDED_
#define ITP_DTYPES_H_INCLUDED_

#include "Bignums.h"
#include "Continuation.h"
#include "PreprocessedTable.h"
#include "TimeSeries.h"

namespace itp {

using HighPrecDouble = bignums::Big_double<12, 24>;
//using HighPrecDouble = boost::multiprecision::mpfr_float;
//using HighPrecDouble = long double;

template <typename T>
using ContinuationsDistribution = Table_with_preproc_info<Continuation<Symbol>,
                                                          std::string, HighPrecDouble, T>;

template <typename T>
using Forecast = Table_with_preproc_info<std::string, size_t, Forecast_point<T>, T>;

template <typename T>
using SymbolsDistributions = Table_with_preproc_info<Symbol, std::string, HighPrecDouble, T>;
} // of itp

#endif // ITP_DTYPES_H_INCLUDED_
