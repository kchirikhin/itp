#ifndef ITP_DTYPES_H_INCLUDED_
#define ITP_DTYPES_H_INCLUDED_

#include "Bignums.h"
#include "Continuation.h"
#include "PreprocessedTable.h"
#include "TimeSeries.h"

namespace itp
{

using HighPrecDouble = bignums::BigDouble<12, 24>;
// using HighPrecDouble = boost::multiprecision::mpfr_float;
// using HighPrecDouble = long double;

template<typename T>
using ContinuationsDistribution = TableWithPreprocInfo<Continuation<Symbol>, std::string, HighPrecDouble, T>;

template<typename T>
using Forecast = TableWithPreprocInfo<std::string, size_t, Forecast_point<T>, T>;

template<typename T>
using SymbolsDistributions = TableWithPreprocInfo<Symbol, std::string, HighPrecDouble, T>;

} // namespace itp

#endif // ITP_DTYPES_H_INCLUDED_
