#pragma once

#include <VoxelEngine/core/core.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(vertex/vertex_buffer.hpp)


namespace ve {
    struct mesh_component {
        shared<gfxapi::vertex_buffer> buffer;
    };
}