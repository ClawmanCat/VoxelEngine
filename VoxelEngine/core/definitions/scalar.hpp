#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>
#include <concepts>


namespace ve {
    using u8  = uint8_t;
    using i8  = int8_t;
    using u16 = uint16_t;
    using i16 = int16_t;
    using u32 = uint32_t;
    using i32 = int32_t;
    using u64 = uint64_t;
    using i64 = int64_t;
    using f32 = float;
    using f64 = double;


    static_assert(
        std::numeric_limits<f32>::is_iec559 && std::numeric_limits<f64>::is_iec559,
        "Platform does not support IEEE 754 floating point numbers."
    );


    template <typename T> requires std::is_floating_point_v<T>
    constexpr T infinity = std::numeric_limits<T>::infinity();

    template <typename T> requires std::is_arithmetic_v<T>
    constexpr T max_value = std::numeric_limits<T>::max();

    template <typename T> requires std::is_arithmetic_v<T>
    constexpr T min_value = std::numeric_limits<T>::lowest();


    template <typename T> constexpr inline bool is_numeric_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
    template <typename T> concept numeric = is_numeric_v<T>;
}