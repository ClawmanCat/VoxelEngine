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


    // Sets the first N bits to one.
    template <typename T> T set_n_bits(u8 count) {
        T result = 0;
        for (u8 i = 0; i < count; ++i) result |= (1 << i);
        return result;
    }


    // Gets the next address that is divisible by alignment.
    // If the current address is divisible by alignment, it is returned.
    // Note: alignment must be a power of two!
    constexpr std::size_t next_aligned_address(std::size_t address, std::size_t alignment) {
        std::size_t offset = address & (alignment - 1);
        return (address - offset) + (alignment * std::size_t(offset != 0));
    }


    template <typename Ctr> requires (std::is_trivial_v<typename Ctr::value_type> && std::contiguous_iterator<typename Ctr::iterator>)
    constexpr std::span<const u8> to_byte_span(const Ctr& ctr) {
        return std::span<const u8> {
            reinterpret_cast<const u8*>(&*ctr.begin()),
            ctr.size() * sizeof(typename Ctr::value_type)
        };
    }


    constexpr u64 nth_bit(u8 n) {
        return 1ull << n;
    }
}