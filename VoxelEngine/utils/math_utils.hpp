#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/glm_traits.hpp>

#include <type_traits>


namespace ve {
    template <typename T> constexpr inline T radians(const T& degrees) {
        return degrees * (T(pi) / T(180));
    }
    
    template <typename T> constexpr inline T degrees(const T& radians) {
        return radians * (T(180) / T(pi));
    }
    
    template <typename T>
    inline T next_multiple_of(const T& value, const T& multiple) {
        return ((value + multiple - 1) / multiple) * multiple;
    }
}