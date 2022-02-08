#ifndef COMPNAMES_H_INCLUDED
#define COMPNAMES_H_INCLUDED

#include "Types.h"

namespace itp
{

CompressorNames SplitConcatenatedNames(const ConcatenatedCompressorNames& concatenated_names, char separator = '_');

/**
 * \brief Splits vector of strings, in which each string consists of compressors's names separated by the
 * separator character, to a vector of vectors with compressors names.
 *
 * \param[in] concatenated_names_vec Vector of concatenated compressor's names.
 * \param[in] separator Names in concatenated_names_vec should be separated by that character.
 *
 * \return Vector of vectors of names, split by the separator.
 */
CompressorNamesVec SplitConcatenatedNames(
	const ConcatenatedCompressorNamesVec& concatenated_names_vec,
	char separator = '_');

/**
 * Concatenates vector of compressor names to a single name using separator.
 *
 * \param[in] compressor_names Vector of compressor's names.
 * \param[in] separator Separator for names in a single string.
 *
 * \return Concatenated name.
 */
ConcatenatedCompressorNames ToConcatenatedCompressorNames(
	const CompressorNames& compressor_names,
	char separator = '_');

/**
 * \brief Find all distinct compressor's names in the vector of vectors of names.
 *
 * \param[in] compressor_names_vec Vector of vector of names.
 *
 * \return Vector of unique names.
 */
CompressorNames FindAllDistinctNames(const CompressorNamesVec& compressor_names_vec);

} // namespace itp

#endif // COMPNAMES_H_INCLUDED
