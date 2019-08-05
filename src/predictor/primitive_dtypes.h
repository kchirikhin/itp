#ifndef PRIMITIVE_DTYPES_INCLUDED
#define PRIMITIVE_DTYPES_INCLUDED

#include "bignums.h"

// #include <boost/multiprecision/mpfr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cfloat>

#define _S(text) std::string(text)

namespace itp {
    using Symbol_t = unsigned char;
    //using Prec_double_t = Big<TTMATH_BITS(64), TTMATH_BITS(128)>;
    using Prec_double_t = bignums::Big_double<12, 24>;
    //using Prec_double_t = boost::multiprecision::mpfr_float;
    //using Prec_double_t = long double;
    using Double_t = Prec_double_t;
    

    template <typename T>
    using Plain_tseries = std::vector<T>;
    
    using Group = std::vector<std::string>;
    using Names = std::vector<std::string>;
} // of itp

#endif // PRIMITIVE_DTYPES_INCLUDED
