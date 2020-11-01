//
// Created by kon on 22.08.2020.
//

#ifndef ITP_MACRO_H
#define ITP_MACRO_H

#include <type_traits>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(std::remove_all_extents_t<decltype(x)>))

// todo: maybe can be generalized with variadic template?
#define UNUSED(x) ((void)x)

#endif //ITP_MACRO_H
