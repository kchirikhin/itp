#ifndef ITP_ICOMPRESSOR_H
#define ITP_ICOMPRESSOR_H

#include "Types.h"

namespace itp
{

/**
 * Base class for all data compression algorithms. Hides calls for library-specific functions.
 */
class ICompressor
{
public:
	using SizeInBits = size_t;
	using Continuations = std::vector<Continuation<Symbol>>;

	virtual ~ICompressor() = default;

	/**
	 * Compresses data and returns size of the output sequence.
	 *
	 * \param[in] data Data to compress.
	 * \param[in] size Size of the data.
	 * \param[out] output_buffer Where to put the result.
	 *
	 * \return Size of the compressed data in bits.
	 */
	virtual SizeInBits Compress(
		const unsigned char* data,
		size_t size,
		std::vector<unsigned char>* output_buffer) = 0;

	/**
	 * Compresses each passed trajectory after the historical values and returns the code lengths for each trajectory.
	 *
	 * \param[in] historical_values Time series.
	 * \param[in] possible_endings Continuations to compress.
	 *
	 * \return Code lengths in bits for each trajectory.
	 */
	virtual std::vector<SizeInBits> CompressContinuations(
		const std::vector<Symbol>& historical_values,
		const Continuations& possible_endings) = 0;

	/**
	 * Inform algorithm about the minimal and maximal possible values in data.
	 *
	 * \param[in] alphabet_min_symbol Minimal value in the data.
	 * \param[in] alphabet_max_symbol Maximal value in the data.
	 */
	virtual void SetTsParams(Symbol alphabet_min_symbol, Symbol alphabet_max_symbol) = 0;
};

} // namespace itp

#endif // ITP_ICOMPRESSOR_H
