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


    template <typename Ctr> requires (std::is_trivial_v<typename Ctr::value_type> && std::contiguous_iterator<typename Ctr::iterator>)
    constexpr std::span<const u8> to_byte_span(const Ctr& ctr) {
        return std::span<const u8> {
            reinterpret_cast<const u8*>(&*ctr.begin()),
            ctr.size() * sizeof(typename Ctr::value_type)
        };
    }
}