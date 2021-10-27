#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Explicit copy.
    template <typename T> constexpr T copy(const T& val) {
        return T { val };
    }
}