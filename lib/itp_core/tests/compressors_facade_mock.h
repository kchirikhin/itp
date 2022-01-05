/**
 * File contains implementation of a mock object, which allows to replace time consuming data compression operation
 * with a predefined values.
 */

#ifndef ITP_COMPRESSORS_FACADE_MOCK_H_INCLUDED_
#define ITP_COMPRESSORS_FACADE_MOCK_H_INCLUDED_

#include "../src/compressors.h"

namespace itp
{

class CompressorsFacadeMock : public CompressorsFacade
{
public:
	MOCK_METHOD2(RegisterCompressor, void(std::string name, std::unique_ptr<ICompressor> compressor));
	MOCK_METHOD3(Compress, ICompressor::SizeInBits(const std::string&, const unsigned char *, size_t));
	MOCK_METHOD3(CompressContinuations, std::vector<ICompressor::SizeInBits>(
		const std::string&,
		const std::vector<Symbol>&,
		const ICompressor::Continuations&));
	MOCK_METHOD1(SetAlphabetDescription, void(AlphabetDescription));
};

} // namespace itp

#endif //ITP_COMPRESSORS_FACADE_MOCK_H_INCLUDED_
