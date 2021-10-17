#pragma once

#include <VoxelEngine/core/core.hpp>

#include <numbers>


namespace ve {
    namespace constants {
        template<typename T>
        constexpr inline T pi = std::numbers::pi_v<T>;

        constexpr inline float f32_pi = pi<f32>;
        constexpr inline float f64_pi = pi<f64>;


        template<typename T>
        constexpr inline T e = std::numbers::e_v<T>;

        constexpr inline float f32_e = e<f32>;
        constexpr inline float f64_e = e<f64>;
    }


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