#pragma once

#include <VoxelEngine/core/core.hpp>

#include <array>


namespace ve {
    using direction = vec3b;
    
    struct directions {
        directions(void) = delete;
        
        constexpr static direction SOUTH = { -1, +0, +0 };
        constexpr static direction NORTH = { +1, +0, +0 };
        constexpr static direction DOWN  = { +0, -1, +0 };
        constexpr static direction UP    = { +0, +1, +0 };
        constexpr static direction WEST  = { +0, +0, -1 };
        constexpr static direction EAST  = { +0, +0, +1 };
        
        [[nodiscard]] constexpr static auto values(void) noexcept {
            return std::array { NORTH, SOUTH, UP, DOWN, EAST, WEST };
        }
    };
}