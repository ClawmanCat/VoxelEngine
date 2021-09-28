#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    using RGBA8 = vec4ub;
    static_assert(sizeof(RGBA8) == 4 * sizeof(u8));


    struct image_rgba8 {
        std::vector<RGBA8> data;
        vec2ui size;
    };
}