#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    constexpr inline std::size_t next_aligned_address(std::size_t address, std::size_t alignment) {
        return ((address / alignment) + 1) * alignment;
    }
}