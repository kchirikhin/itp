#ifndef ITP_INONCOMPRESSIONALGORITHM_H
#define ITP_INONCOMPRESSIONALGORITHM_H

#include "../../src/ICompressor.h"

#include <memory>


namespace itp
{

enum class ConfidenceLevel : unsigned char
{
	// Do not reorder! Python extensions return numeric representations of this values.
	kConfident = 0,
	kNotConfident = 1
};

class INonCompressionAlgorithm
{
public:
	using Guess = std::pair<Symbol, ConfidenceLevel>;

	virtual ~INonCompressionAlgorithm() = default;
	virtual Guess GiveNextPrediction(const unsigned char* data, size_t size) = 0;
	virtual void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) = 0;
};
using INonCompressionAlgorithmPtr = std::unique_ptr<INonCompressionAlgorithm>;

} // namespace itp

#endif //ITP_INONCOMPRESSIONALGORITHM_H
