#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    namespace direction {
        constexpr inline vec3i BACKWARD = vec3i { +0, +0, +1 };
        constexpr inline vec3i FORWARD  = vec3i { +0, +0, -1 };
        constexpr inline vec3i UP       = vec3i { +0, +1, +0 };
        constexpr inline vec3i DOWN     = vec3i { +0, -1, +0 };
        constexpr inline vec3i RIGHT    = vec3i { +1, +0, +0 };
        constexpr inline vec3i LEFT     = vec3i { -1, +0, +0 };

        constexpr inline vec3i NORTH = FORWARD;
        constexpr inline vec3i SOUTH = BACKWARD;
        constexpr inline vec3i EAST  = RIGHT;
        constexpr inline vec3i WEST  = LEFT;
    }


    constexpr inline std::array directions {
        direction::BACKWARD,
        direction::FORWARD,
        direction::UP,
        direction::DOWN,
        direction::RIGHT,
        direction::LEFT
    };

    using direction_t = u8; // Indexes into the above array.


    constexpr inline direction_t opposing_direction(direction_t dir) {
        // Every 2n and 2n + 1 index is an opposing direction pair, so just flip the last bit to get the opposing direction.
        return dir ^ 1;
    }


    constexpr inline direction_t direction_from_vector(const vec3i& vec) {
        // LSB contains the direction of the set axis (1 for -1 or 0 for 1),
        // other two bits store whether the non-zero axis was in the Y or X direction.
        return direction_t(
            bool(vec.x + vec.y + vec.z - 1) |
            (bool(vec.y) << 1) |
            (bool(vec.x) << 2)
        );
    }
}