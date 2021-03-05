#pragma once

#include <VoxelEngine/core/core.hpp>

#include <limits>


namespace ve {
    template <typename T> constexpr inline T radians(const T& degrees) {
        return degrees * (T(pi) / T(180));
    }
    
    template <typename T> constexpr inline T degrees(const T& radians) {
        return radians * (T(180) / T(pi));
    }
    
    template <typename T> constexpr inline bool in(const T& val, const T& min, const T& max) {
        return val >= min && val < max;
    }
    
    
    template <typename T> constexpr T max_value = std::numeric_limits<T>::max();
    template <typename T> constexpr T min_value = std::numeric_limits<T>::lowest();
    template <typename T> constexpr T infinity  = std::numeric_limits<T>::infinity();
}