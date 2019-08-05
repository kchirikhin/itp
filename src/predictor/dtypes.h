#ifndef DTYPES_INCLUDED
#define DTYPES_INCLUDED

#include "continuation.h"
#include "ftable.h"
#include "tseries.h"

namespace itp {
    template <typename T>
    using Continuations_distribution = Table_with_preproc_info<Continuation<Symbol_t>,
                                                               std::string, Prec_double_t, T>;

    template <typename T>
    using Forecast = Table_with_preproc_info<std::string, size_t, Forecast_point<Double_t>, T>;

    template <typename T>
    using Symbols_distributions = Table_with_preproc_info<Symbol_t, std::string, Prec_double_t, T>;
} // of itp

#endif // DTYPES_INCLUDED
