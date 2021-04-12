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
    constexpr inline direction end(direction)   { return direction(((u8) direction::WEST) << 1);  }
    
    constexpr inline void operator++(direction& d) { d = direction(((u8) d) << 1); }
    constexpr inline void operator--(direction& d) { d = direction(((u8) d) >> 1); }
    constexpr inline direction operator* (direction d) { return d; }
    
    
    // Convert a direction to a normalized vector pointing in that direction.
    constexpr inline vec3i direction_vector(direction dir) {
        vec3i result { 0 };
        
        result.x -= bool(dir & direction::WEST);
        result.x += bool(dir & direction::EAST);
        result.y -= bool(dir & direction::DOWN);
        result.y += bool(dir & direction::UP);
        result.z -= bool(dir & direction::SOUTH);
        result.z += bool(dir & direction::NORTH);
        
        return result;
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
        vec3i difference = to - from;
        
        return direction(
            (bool(difference.z) << (0 + (difference.z < 0))) +
            (bool(difference.y) << (2 + (difference.y < 0))) +
            (bool(difference.x) << (4 + (difference.z < 0)))
        );
    }
}