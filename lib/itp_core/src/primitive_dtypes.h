#ifndef ITP_PRIMITIVE_DTYPES_H_INCLUDED_
#define ITP_PRIMITIVE_DTYPES_H_INCLUDED_

#include <cfloat>
#include <iostream>
#include <string>
#include <valarray>
#include <vector>

// #define _S(text) std::string(text)

namespace itp {

using Symbol = unsigned char;
using VectorSymbol = std::valarray<Symbol>;

using Double = double;
//using VectorDouble = std::valarray<Double>;

class VectorDouble : public std::valarray<double> {
 public:
  using std::valarray<double>::valarray;
  
  VectorDouble(const VectorSymbol &vs) {
    resize(vs.size());
    for (size_t i = 0; i < size(); ++i) {
      operator[](i) = vs[i];
    }
  }
};

inline std::ostream& operator << (std::ostream &ost, const VectorSymbol &vs) {
  ost << '{';
  for (size_t i = 0; i < vs.size() - 1; ++i) {
    ost << vs[i] << ", ";
  }
  if (vs.size() > 0) {
    ost << vs[vs.size() - 1];
  }
  
  return ost << '}';
}

inline std::ostream& operator << (std::ostream &ost, const VectorDouble &vd) {
  ost << '{';
  for (size_t i = 0; i < vd.size() - 1; ++i) {
    ost << vd[i] << ", ";
  }
  if (vd.size() > 0) {
    ost << vd[vd.size() - 1];
  }
  
  return ost << '}';
}

template <typename T>
using PlainTimeSeries = std::vector<T>;
    
using Group = std::vector<std::string>;
using Names = std::vector<std::string>;

inline std::string operator "" _s(const char *str, size_t size) {
  return std::string(str, size);
}
} // of itp

#endif // ITP_PRIMITIVE_DTYPES_H_INCLUDED_
