#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx {
    // This struct matches the members used by the struct Camera in common.util.glsl.
    struct camera_uniform {
        mat4f matrix;
        vec3f position;
        float near;
    };
}