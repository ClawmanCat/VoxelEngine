#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <typename T> constexpr inline T radians(const T& degrees) {
        return degrees * (T(pi) / T(180));
    }
    
    template <typename T> constexpr inline T degrees(const T& radians) {
        return radians * (T(180) / T(pi));
    }
}