#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>


namespace ve {
    struct chunk_generator {
        virtual ~chunk_generator(void) = default;
        virtual unique<chunk> generate(const vec3i& where) = 0;
    };
}