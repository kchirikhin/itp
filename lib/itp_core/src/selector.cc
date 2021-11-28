//
// Created by kon on 26.10.2021.
//

#include "selector_private.h"

namespace itp
{

namespace evaluation
{

template<>
std::vector<size_t> ComputeCorrections<Symbol>(const std::vector<size_t>& quanta_counts, const size_t)
{
	return {0};
}

template<>
std::vector<size_t> ComputeCorrections<VectorSymbol>(const std::vector<size_t>& quanta_counts, const size_t)
{
	return {0};
}

template<>
void CheckQuantaCounts<Symbol>(const std::vector<size_t>&)
{
}

template<>
void CheckQuantaCounts<VectorSymbol>(const std::vector<size_t>&)
{
}

bool CompressionResultComparator(const std::pair<std::string, size_t>& lhs, const std::pair<std::string, size_t>& rhs)
{
	if (lhs.second < rhs.second)
	{
		return true;
	}
	else if (rhs.second < lhs.second)
	{
		return false;
	}

	return lhs.first <= rhs.first;
}

itp::Names GetBestCompressors(const std::unordered_map<std::string, size_t>& results_of_computations,
							  const size_t target_number)
{
	if (results_of_computations.size() < target_number)
	{
		throw SelectorError("results_of_computations's size is less than the target_number of compressors");
	}

	std::cout << "Results:\n";
	for (const auto&[name, size]: results_of_computations)
	{
		std::cout << name << ' ' << size << "\n";
	}


	std::vector<std::pair<std::string, size_t>> results_sorted_by_file_size{std::cbegin(results_of_computations),
																			std::cend(results_of_computations)};
	std::sort(std::begin(results_sorted_by_file_size), std::end(results_sorted_by_file_size),
			  CompressionResultComparator);


	itp::Names to_return;
	ad_hoc::for_each_n(std::cbegin(results_sorted_by_file_size), target_number, [&](const auto& name_size_pair)
	{
		to_return.push_back(name_size_pair.first);
	});

	return to_return;
}

} // evaluation

template<typename T>
Names SelectBestCompressors(
	const std::vector<T>& history,
	const std::set<std::string>& compressors,
	const size_t difference,
	const std::vector<size_t>& quanta_count,
	const Share part_to_consider,
	const size_t target_number)
{
	const auto elems_to_consider = static_cast<size_t>(std::ceil(history.size() * part_to_consider));
	const std::vector<T> shrinked_history{std::cbegin(history), std::next(std::cbegin(history), elems_to_consider)};

	using namespace evaluation;
	CodeLengthEvaluator<T> evaluator{MakeStandardCompressorsPool()};

	return GetBestCompressors(evaluator.Evaluate(shrinked_history, compressors, difference, quanta_count),
							  target_number);
}

template Names SelectBestCompressors<Symbol>(
	const std::vector<Symbol>& history,
	const std::set<std::string>& compressors,
	const size_t difference,
	const std::vector<size_t>& quanta_count,
	const Share part_to_consider,
	const size_t target_number);

template Names SelectBestCompressors<Double>(
	const std::vector<Double>& history,
	const std::set<std::string>& compressors,
	const size_t difference,
	const std::vector<size_t>& quanta_count,
	const Share part_to_consider,
	const size_t target_number);

} // itp