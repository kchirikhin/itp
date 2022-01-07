#include "Compnames.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace itp
{

CompressorNames SplitConcatenatedNames(const ConcatenatedCompressorNames& concatenated_names, char separator)
{
	CompressorNames result;
	std::string name;

	std::istringstream iss(concatenated_names);
	while (std::getline(iss, name, separator))
	{
		result.push_back(name);
	}

	return result;
}

CompressorNamesVec SplitConcatenatedNames(
	const ConcatenatedCompressorNamesVec& concatenated_names_vec,
	char separator)
{
	const auto transformator = [separator](const auto& concatenated_names)
	{ return SplitConcatenatedNames(concatenated_names, separator); };

	std::vector<CompressorNames> result;
	std::transform(
		std::cbegin(concatenated_names_vec),
		std::cend(concatenated_names_vec),
		std::back_inserter(result),
		transformator);

	return result;
}

ConcatenatedCompressorNames ToConcatenatedCompressorNames(const CompressorNames& compressors, char separator)
{
	if (compressors.empty())
	{
		return std::string{};
	}

	std::ostringstream oss;
	oss << compressors[0];
	for (size_t i = 1; i < compressors.size(); ++i)
	{
		oss << separator << compressors[i];
	}

	return oss.str();
}

CompressorNames FindAllDistinctNames(const CompressorNamesVec& compressor_names_vec)
{
	std::unordered_set<CompressorName> unique_names;
	std::for_each(
		std::cbegin(compressor_names_vec),
		std::cend(compressor_names_vec),
		[&unique_names](const auto& compressor_names)
		{ unique_names.insert(std::cbegin(compressor_names), std::cend(compressor_names)); });

	return {std::make_move_iterator(std::begin(unique_names)), std::make_move_iterator(std::end(unique_names))};
}

} // namespace itp
