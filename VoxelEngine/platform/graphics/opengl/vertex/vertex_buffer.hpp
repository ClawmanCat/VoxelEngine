#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    struct vertex_buffer {
        virtual ~vertex_buffer(void) = default;
        virtual void draw(const render_context& ctx) = 0;
        virtual GLuint get_id(void) = 0;
    };
}