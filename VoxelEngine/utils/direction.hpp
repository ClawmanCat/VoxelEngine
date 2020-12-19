#pragma once

#include <VoxelEngine/core/core.hpp>

#include <magic_enum.hpp>

#include <array>


namespace ve {
    enum class direction {
        NORTH, SOUTH, UP, DOWN, EAST, WEST
    };
    
    constexpr inline std::size_t num_directions = magic_enum::enum_count<direction>();
    
    
    inline vec3ub direction_vector(direction dir) {
        switch (dir) {
            case direction::SOUTH: return { -1, +0, +0 };
            case direction::NORTH: return { +1, +0, +0 };
            case direction::DOWN:  return { +0, -1, +0 };
            case direction::UP:    return { +0, +1, +0 };
            case direction::WEST:  return { +0, +0, -1 };
            case direction::EAST:  return { +0, +0, +1 };
        }
    }
}