/**
 * Implementation of an algorithm of fast selection of the several most suitable compressors among the specified ones
 * for the time series.
 */

#ifndef ITP_SELECTOR_H_INCLUDED_
#define ITP_SELECTOR_H_INCLUDED_

#include "../../src/primitive_dtypes.h"

#include <set>

namespace itp
{

/**
 * Class that represents a share (like percentage), ranging from 0 to 1. Allows explicit casting from double and
 * implicit casting to double.
 */
class Share
{
public:
	explicit Share(const double init_value = 0.)
			: share_{init_value}
	{
		ThrowIfShareIsIncorrect();
	}

	operator double() const
	{
		return share_;
	}

private:
	void ThrowIfShareIsIncorrect() const
	{
		if (share_ < .0 || 1. < share_)
		{
			throw std::invalid_argument{"share is out of range"};
		}
	}

	double share_;
};

class SelectorError : public std::runtime_error
{
public:
	explicit SelectorError(const std::string& what_arg)
		: runtime_error{what_arg}
	{
		// DO NOTHING
	}
};

template<typename T>
itp::Names SelectBestCompressors(
	const std::vector<T>& history,
	const std::set<std::string>& compressors,
	const size_t difference,
	const std::vector<size_t>& quanta_count,
	const Share part_to_consider,
	const size_t target_number);

} // namespace itp

#endif // ITP_SELECTOR_H_INCLUDED_
