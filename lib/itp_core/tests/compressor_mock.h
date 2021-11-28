#ifndef ITP_COMPRESSOR_MOCK_H_INCLUDED_
#define ITP_COMPRESSOR_MOCK_H_INCLUDED_

#include <gmock/gmock.h>

#include "../src/ICompressor.h"

namespace itp
{

class CompressorMock : public ICompressor
{
public:
	MOCK_METHOD3(Compress, size_t(const unsigned char*, size_t, std::vector<unsigned char>*));
	MOCK_METHOD2(CompressEndings, std::vector<size_t>(const std::vector<Symbol>&, const Trajectories&));
	MOCK_METHOD2(SetTsParams, void(Symbol, Symbol));
};

} // namespace itp

#endif // ITP_COMPRESSOR_MOCK_H_INCLUDED_
