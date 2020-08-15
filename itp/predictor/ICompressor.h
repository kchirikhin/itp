#ifndef ITP_ICOMPRESSOR_H
#define ITP_ICOMPRESSOR_H

#include "dtypes.h"

namespace itp
{

/**
 * Base class for all data compression algorithms. Hides calls for library-specific functions.
 */
class ICompressor
{
public:
	virtual ~ICompressor() = default;

	/**
	 * Compresses data and returns size of the output sequence.
	 * @param data Data to compress.
	 * @param size Size of data.
	 * @param output_buffer Where to put result.
	 * @return Size of the compressed data in bytes.
	 */
	virtual size_t operator()(const unsigned char* data, size_t size,
							  std::vector<unsigned char>* output_buffer) = 0;

	/**
	 * Inform algorithm about minimal and maximal possible values in data. It's important for automaton.
	 */
	virtual void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) = 0;
};

} // namespace itp

#endif //ITP_ICOMPRESSOR_H
