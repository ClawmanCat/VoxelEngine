#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve {
    template <typename T> requires std::is_integral_v<T>
    [[nodiscard]] constexpr inline u8 bit_count(void) noexcept {
        return 8 * sizeof(T);
    }
    
    
    // Gets the value of the most significant bit.
    template <typename T> requires std::is_integral_v<T>
    [[nodiscard]] constexpr inline u8 get_msb(T value) noexcept {
        return value >> (bit_count<T>() - 1);
    }
    
    
    // Sets the value of the most significant bit.
    template <typename T> requires std::is_integral_v<T>
    [[nodiscard]] constexpr inline T set_msb(T value) noexcept {
        return value | (T(1) << bit_count<T>() - 1);
    }
    
    
    // Clears the value of the most significant bit.
    template <typename T> requires std::is_integral_v<T>
    [[nodiscard]] constexpr inline T clear_msb(T value) noexcept {
        return value & ~(T(1) << bit_count<T>() - 1);
    }
}