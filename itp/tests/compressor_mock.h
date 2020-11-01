#ifndef ITP_COMPRESSOR_MOCK_H_INCLUDED_
#define ITP_COMPRESSOR_MOCK_H_INCLUDED_

#include <gmock/gmock.h>

#include <ICompressor.h>

namespace itp
{

class CompressorMock : public ICompressor
{
public:
	MOCK_METHOD3(CallOperator, size_t(const unsigned char*, size_t, std::vector<unsigned char>*));
	MOCK_METHOD2(SetTsParams, void(Symbol, Symbol));

	size_t operator()(const unsigned char* data, size_t size, std::vector<unsigned char>* output_buffer) override
	{
		return CallOperator(data, size, output_buffer);
	}
};

} // namespace itp

#endif // ITP_COMPRESSOR_MOCK_H_INCLUDED_
