#ifndef FTABLE_H_INCLUDED
#define FTABLE_H_INCLUDED

#include "DataFrame.h"
#include "PreprocInfo.h"
#include <iostream>

namespace itp
{

template<typename Index, typename Factor, typename TableValue, typename OrigValue>
class TableWithPreprocInfo
	: public DataFrame<Index, Factor, TableValue>
	, public PreprocInfo<OrigValue>
{
public:
	using DataFrame<Index, Factor, TableValue>::DataFrame;
};

template<typename Index, typename Factor, typename TableValue, typename OrigValue>
std::ostream& operator<<(
	std::ostream& ost,
	const TableWithPreprocInfo<Index, Factor, TableValue, OrigValue>& table)
{
	for (const auto& index : table.get_index())
	{
		ost << index;
		for (const auto& factor : table.get_factors())
		{
			ost << ' ' << table(index, factor).point;
		}
		ost << '\n';
	}

	return ost;
}
} // namespace itp

#endif // FTABLE_H_INCLUDED
