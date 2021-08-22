#ifndef ITP_NONCOMPRESSIONALGORITHMMOCK_H
#define ITP_NONCOMPRESSIONALGORITHMMOCK_H

#include <gmock/gmock.h>

#include "INonCompressionAlgorithm.h"

namespace itp::mocks
{

class NonCompressionAlgorithmMock : public INonCompressionAlgorithm
{
public:
	MOCK_METHOD((std::pair<Symbol, ConfidenceLevel>), GiveNextPrediction, (const unsigned char* data, size_t size), (override));
	MOCK_METHOD(void, SetTsParams, (Symbol alphabet_min_symbol, Symbol alphabet_max_symbol), (override));
};

} // namespace itp::mocks

#endif //ITP_NONCOMPRESSIONALGORITHMMOCK_H
