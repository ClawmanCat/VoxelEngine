#pragma once

#include <VoxelEngine/core/core.hpp>

#include <bit>


namespace ve {
    template <typename T> requires std::is_unsigned_v<T>
    constexpr u8 msb(T value) {
        return (8 * sizeof(T)) - std::countl_zero(value);
    }
    
    
    template <typename T> requires std::is_unsigned_v<T>
    constexpr u8 lsb(T value) {
        return std::countr_zero(value);
    }
}