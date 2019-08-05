#ifndef FTABLE_H_INCLUDED
#define FTABLE_H_INCLUDED

#include "dframe.h"
#include "preproc_info.h"

namespace itp {
    template <typename Index, typename Factor, typename Table_value, typename Orig_value>
    class Table_with_preproc_info : public Data_frame<Index, Factor, Table_value>, public Preproc_info<Orig_value> {
    public:
        using Data_frame<Index, Factor, Table_value>::Data_frame;
    };
} // of itp

#endif // FTABLE_H_INCLUDED
