#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/blend.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    // Note: clear color and depth are set on a per-target basis, and can be found in render_target_settings instead.
    struct pipeline_settings {
        enum class topology_t : GLenum {
            POINTS         = GL_POINTS,
            LINES          = GL_LINES,
            TRIANGLES      = GL_TRIANGLES,
            TRIANGLE_FAN   = GL_TRIANGLE_FAN,
            TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
            PATCHES        = GL_PATCHES
        } topology = topology_t::TRIANGLES;


        enum class polygon_mode_t : GLenum {
            SURFACE  = GL_FILL,
            EDGES    = GL_LINE,
            VERTICES = GL_POINT
        } polygon_mode = polygon_mode_t::SURFACE;


        enum class cull_mode_t : GLenum {
            NO_CULLING = 0,
            CULL_FRONT = GL_FRONT,
            CULL_BACK  = GL_BACK,
            CULL_BOTH  = GL_FRONT_AND_BACK
        } cull_mode = cull_mode_t::NO_CULLING;


        enum class cull_direction_t : GLenum {
            CLOCKWISE         = GL_CW,
            COUNTER_CLOCKWISE = GL_CCW
        } cull_direction = cull_direction_t::COUNTER_CLOCKWISE;


        float    line_width     = 1.0f;
        bool     depth_testing  = true;
        bool     depth_clamp    = true;
        fn<void> blend_function = blend_functions::normal;
    };
}