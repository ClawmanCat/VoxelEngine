#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/bit.hpp>

#include <array>


namespace ve {
    enum class direction : u8 {
        NONE  = 0,
        NORTH = (1 << 0),
        SOUTH = (1 << 1),
        UP    = (1 << 2),
        DOWN  = (1 << 3),
        EAST  = (1 << 4),
        WEST  = (1 << 5)
    };
    
    VE_BITWISE_ENUM(direction);
    
    constexpr inline std::size_t num_directions = 6;
    
    
    // Allow foreach direction constructs.
    constexpr inline direction begin(direction) { return direction::NORTH; }
    constexpr inline direction end(direction)   { return direction::WEST;  }
    
    constexpr inline void operator++(direction& d) { d = direction(((u8) d) << 1); }
    constexpr inline void operator--(direction& d) { d = direction(((u8) d) >> 1); }
    constexpr inline direction operator* (direction d) { return d; }
    
    
    // Convert a direction to a normalized vector pointing in that direction.
    constexpr inline vec3ub direction_vector(direction dir) {
        std::array<vec3ub, 6> vectors {
            vec3ub { -1, +0, +0 }, vec3ub { +1, +0, +0 },
            vec3ub { +0, -1, +0 }, vec3ub { +0, +1, +0 },
            vec3ub { +0, +0, -1 }, vec3ub { +0, +0, +1 }
        };
        
        return vectors[least_significant_bit((u8) dir)];
    }
    
    
    // Get the direction opposite of the given direction.
    constexpr inline direction opposing(direction dir) {
        constexpr std::array opposing_directions {
            direction::SOUTH, direction::NORTH,
            direction::DOWN,  direction::UP,
            direction::WEST,  direction::EAST
        };
        
        return opposing_directions[least_significant_bit((u8) dir)];
    }
    
    
    // Gets the direction one would travel in to get from a to b,
    // assuming a and b only differ their coordinate on one axis.
    inline direction heading(const vec3i& from, const vec3i& to) {
        u8 index =
            (from.x < to.x) +
            ((from.y != to.y) << 1) + (from.y < to.y) +
            ((from.z != to.z) << 2) + (from.z < to.z);
        
        return direction(1 << index);
    }
}