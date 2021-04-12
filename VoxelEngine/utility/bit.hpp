#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    constexpr inline std::size_t next_aligned_address(std::size_t address, std::size_t alignment) {
        return ((address / alignment) + 1) * alignment;
    }
    
    
    template <typename T>
    constexpr inline u8 least_significant_bit(T value) {
        #if defined(VE_COMPILER_CLANG) || defined(VE_COMPILER_GCC)
            return __builtin_ctz(value);
        #else
            // Adapted from:
            // Leiserson, C. E., Prokop, H., & Randall, K. H. (1998).
            // Using de Bruijn sequences to index a 1 in a computer word.
            
            if constexpr (sizeof(T) > sizeof(u32)) {
                static_assert(sizeof(T) == sizeof(u64));
                
                u8 high = least_significant_bit((u32) ((value & 0xFFFFFFFF00000000) >> 32));
                u8 low  = least_significant_bit((u32)  (value & 0x00000000FFFFFFFF));
                
                constexpr u8 mask[2] = { 0xFF, 0X00 };
                return low + ((high + 32) & mask[low > 0]);
            } else {
                constexpr u8 positions[32] = {
                    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
                    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
                };
            
                return positions[((u32)((value & -value) * 0x077CB531U)) >> 27];
            }
        #endif
    }
}