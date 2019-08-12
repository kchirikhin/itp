#ifndef ITP_PRIMITIVE_DTYPES_H_INCLUDED_
#define ITP_PRIMITIVE_DTYPES_H_INCLUDED_

#include "bignums.h"

// #include <boost/multiprecision/mpfr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cfloat>

#define _S(text) std::string(text)

namespace itp {

using Symbol = unsigned char;

//using HighPrecDouble = Big<TTMATH_BITS(64), TTMATH_BITS(128)>;
using HighPrecDouble = bignums::Big_double<12, 24>;
//using HighPrecDouble = boost::multiprecision::mpfr_float;
//using HighPrecDouble = long double;
using Double = HighPrecDouble;
    

template <typename T>
using PlainTimeSeries = std::vector<T>;
    
using Group = std::vector<std::string>;
using Names = std::vector<std::string>;
} // of itp

#endif // ITP_PRIMITIVE_DTYPES_H_INCLUDED_
