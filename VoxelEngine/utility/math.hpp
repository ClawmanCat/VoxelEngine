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


    template <typename T, typename R>
    constexpr bool in_range(T value, R min, R max) {
        return value >= min && value < max;
    }


    template <typename T> constexpr T square(T value) {
        return value * value;
    }


    template <typename T> constexpr T cube(T value) {
        return value * value * value;
    }


    template <typename T>
    constexpr inline T flatten(const vec3<T>& pos, T size) {
        return pos.z + (pos.y * size) + (pos.x * square(size));
    }


    template <typename T>
    constexpr inline vec3<T> unflatten(T pos, T size) {
        T x = pos / square(size);
        T y = (pos / size) % size;
        T z = pos % size;

        return { x, y, z };
    }
}