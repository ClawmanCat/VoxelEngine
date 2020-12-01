#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>
#include <bit>


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
    
    
    // Checks if a number is a power of two.
    template <typename T> requires std::is_integral_v<T>
    [[nodiscard]] constexpr inline bool is_power_of_2(T value) noexcept {
        return std::popcount(value) == 1;
    }
    
    
    // Gets the first power of two larger than the given value.
    template <typename T> requires std::is_integral_v<T>
    [[nodiscard]] constexpr inline T next_power_of_2(T value) noexcept {
        --value;
        
        for (T i = T(1); i < sizeof(T) * 8; i <<= 1) {
            value |= value >> i;
        }
        
        return ++value;
    }
}