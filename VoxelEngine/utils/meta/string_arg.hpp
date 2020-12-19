#pragma once

#include <VoxelEngine/core/core.hpp>

#include <algorithm>


namespace ve::meta {
    template <std::size_t N> struct string_arg {
        constexpr string_arg (ArrayRef<const char, N> c_string) {
            std::copy_n(c_string, N, this->c_string);
        }
        
        char c_string[N];
    };
}