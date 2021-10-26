//
// Created by kon on 26.10.2021.
//

#include "selector_private.h"

namespace itp
{

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