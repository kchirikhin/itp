/**
 * File contains implementation of a mock object, which allows to replace time consuming data compression operation
 * with a predefined values.
 */

#ifndef ITP_COMPRESSORS_FACADE_MOCK_H_INCLUDED_
#define ITP_COMPRESSORS_FACADE_MOCK_H_INCLUDED_

#include <compressors.h>

namespace itp
{

class CompressorsFacadeMock : public CompressorsFacade
{
public:
	MOCK_CONST_METHOD3(Compress, size_t(const std::string&, const unsigned char *, size_t));
	MOCK_METHOD1(ResetAlphabetDescription, void(AlphabetDescription));
};

} // namespace itp

#endif //ITP_COMPRESSORS_FACADE_MOCK_H_INCLUDED_
