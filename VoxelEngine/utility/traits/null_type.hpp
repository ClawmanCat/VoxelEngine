#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>


namespace ve::meta {
    struct null_type {};
    
    
    // Allow multiple null-type bases at once.
    template <string_arg Name> struct unique_null_type {};
}