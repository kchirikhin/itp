#ifndef COMPNAMES_H_INCLUDED
#define COMPNAMES_H_INCLUDED

#include "Types.h"

namespace itp {
    Names split_concatenated_names(std::string concatenated_names, char separator = '_');

    /**
     * Splits vector of strings, in which each string consists of compressors's names
     * separated by the separator character,
     * to a vector of vectors with compressors names.
     *
     * @param concatenated_names Vector of concatenated compressor's names.
     * @param separator Names in concatenated_names should be separated by that
     * character.
     *
     * @return Vector of vectors of names, splitted by the separator.
     */
    std::vector<Names> split_concatenated_names(const std::vector<std::string>
                                                &concatenated_names,
                                                char separator = '_');

    /**
     * Concatenates vector of names to a single name using separator.
     *
     * @param compressors Vector of compressor's names.
     * @param separator Separator for names in a single string.
     *
     * @return Concatenated name.
     */
    std::string concatenate(const Names &compressors, char separator = '_');

    /**
     * Find all distinct compressor's name in the vector of vectors of names.
     *
     * @param compressors Vector of vector of names.
     *
     * @return Vector of unique names.
     */
    Names find_all_distinct_names(const std::vector<Names> &compressors);
} // of itp

#endif // COMPNAMES_H_INCLUDED
