#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <typename T> requires std::is_signed_v<T>
    constexpr T sign(T value) {
        return (value > 0) - (value < 0);
    }


    template <typename T> requires std::is_integral_v<T>
    constexpr T pow(T base, T exp) {
        if (exp == 0) return T(1);
        if (exp == 1) return base;

        T half = pow(base, exp >> 1);
        return (exp & 1) ? (base * half * half) : (half * half);
    }
}