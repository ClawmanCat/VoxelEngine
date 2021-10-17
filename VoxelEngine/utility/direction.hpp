#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    namespace direction {
        constexpr inline vec3f UP       = vec3f { +0, +1, +0 };
        constexpr inline vec3f DOWN     = vec3f { +0, -1, +0 };
        constexpr inline vec3f BACKWARD = vec3f { +0, +0, +1 };
        constexpr inline vec3f FORWARD  = vec3f { +0, +0, -1 };
        constexpr inline vec3f RIGHT    = vec3f { +1, +0, +0 };
        constexpr inline vec3f LEFT     = vec3f { -1, +0, +0 };

        constexpr inline vec3f NORTH = FORWARD;
        constexpr inline vec3f SOUTH = BACKWARD;
        constexpr inline vec3f EAST  = RIGHT;
        constexpr inline vec3f WEST  = LEFT;
    }


    constexpr inline std::array directions {
        direction::UP,
        direction::DOWN,
        direction::FORWARD,
        direction::BACKWARD,
        direction::RIGHT,
        direction::LEFT
    };
}