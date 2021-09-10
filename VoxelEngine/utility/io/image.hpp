#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    struct image {
        std::vector<u8> data;
        vec2ui size;

        u8 channels, stride;
        // No formats have more than 4 channels.
        std::array<u8, 4> channel_depths;
    };
}