#ifndef ITP_PRIMITIVE_DTYPES_H_INCLUDED_
#define ITP_PRIMITIVE_DTYPES_H_INCLUDED_

#include <cfloat>
#include <iostream>
#include <string>
#include <valarray>
#include <vector>


namespace itp
{

using Symbol = unsigned char;
using VectorSymbol = std::valarray<Symbol>;

using Double = double;
// using VectorDouble = std::valarray<Double>;

class VectorDouble : public std::valarray<double>
{
public:
	using std::valarray<double>::valarray;

	VectorDouble(const VectorSymbol& vs)
	{
		resize(std::size(vs));
		for (size_t i = 0; i < size(); ++i)
		{
			operator[](i) = vs[i];
		}
	}
};

inline std::ostream& operator<<(std::ostream& ost, const VectorSymbol& vs)
{
	ost << '{';
	for (size_t i = 0; i < std::size(vs) - 1; ++i)
	{
		ost << vs[i] << ", ";
	}
	if (std::size(vs) > 0)
	{
		ost << vs[std::size(vs) - 1];
	}

	return ost << '}';
}

inline std::ostream& operator<<(std::ostream& ost, const VectorDouble& vd)
{
	ost << '{';
	for (size_t i = 0; i < std::size(vd) - 1; ++i)
	{
		ost << vd[i] << ", ";
	}
	if (std::size(vd) > 0)
	{
		ost << vd[std::size(vd) - 1];
	}

	return ost << '}';
}

template<typename T>
using PlainTimeSeries = std::vector<T>;

using CompressorName = std::string;
using CompressorNames = std::vector<CompressorName>;
using CompressorNamesVec = std::vector<CompressorNames>;
using ConcatenatedCompressorNames = std::string;
using ConcatenatedCompressorNamesVec = std::vector<ConcatenatedCompressorNames>;

inline std::string operator"" _s(const char* str, size_t size)
{
	return std::string(str, size);
}

} // namespace itp

#endif // ITP_PRIMITIVE_DTYPES_H_INCLUDED_
