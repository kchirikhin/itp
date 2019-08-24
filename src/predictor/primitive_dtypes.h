#ifndef ITP_PRIMITIVE_DTYPES_H_INCLUDED_
#define ITP_PRIMITIVE_DTYPES_H_INCLUDED_

#include "bignums.h"

// #include <boost/multiprecision/mpfr.hpp>

#include <cfloat>
#include <iostream>
#include <string>
#include <valarray>
#include <vector>

#define _S(text) std::string(text)

namespace itp {

using Symbol = unsigned char;
using VectorSymbol = std::valarray<Symbol>;

using HighPrecDouble = bignums::Big_double<12, 24>;
//using HighPrecDouble = boost::multiprecision::mpfr_float;
//using HighPrecDouble = long double;

using Double = double;
using VectorDouble = std::valarray<Double>;
    

template <typename T>
using PlainTimeSeries = std::vector<T>;
    
using Group = std::vector<std::string>;
using Names = std::vector<std::string>;
} // of itp

#endif // ITP_PRIMITIVE_DTYPES_H_INCLUDED_
