#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    // Prevent automatic deduction of function template arguments from parameters passed using this type.
    template <typename T> using dont_deduce = std::common_type_t<T>;
}