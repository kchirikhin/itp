#ifndef ITP_INONCOMPRESSIONALGORITHM_H
#define ITP_INONCOMPRESSIONALGORITHM_H

#include "ICompressor.h"

#include <memory>


namespace itp
{

enum class ConfidenceLevel : unsigned char
{
	kConfident,
	kNotConfident
};

class INonCompressionAlgorithm
{
public:
	virtual void RegisterFullTimeSeries(const unsigned char* data, size_t size) = 0;
	virtual std::pair<Symbol, ConfidenceLevel> GiveNextPrediction() = 0;
	virtual void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) = 0;
};
using INonCompressionAlgorithmPtr = std::unique_ptr<INonCompressionAlgorithm>;

} // namespace itp

#endif //ITP_INONCOMPRESSIONALGORITHM_H
