#ifndef FTABLE_H_INCLUDED
#define FTABLE_H_INCLUDED

#include "DataFrame.h"
#include "PreprocInfo.h"
#include <iostream>

namespace itp {

template <typename Index, typename Factor, typename Table_value, typename Orig_value>
class Table_with_preproc_info : public Data_frame<Index, Factor, Table_value>, public Preproc_info<Orig_value> {
 public:
  using Data_frame<Index, Factor, Table_value>::Data_frame;
};

template <typename Index, typename Factor, typename Table_value, typename Orig_value>
std::ostream& operator << (std::ostream & ost,
                           const Table_with_preproc_info<Index, Factor, Table_value, Orig_value> &table) {
  for (const auto &index : table.get_index()) {
    ost << index;
    for (const auto &factor : table.get_factors()) {
      ost << ' ' << table(index, factor).point;
    }
    ost << '\n';
  }
  
  return ost;
}
} // of itp

#endif // FTABLE_H_INCLUDED
