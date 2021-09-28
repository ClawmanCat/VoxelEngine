#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    template <typename T> inline void bind_uniform(const char* name, const T& value, const render_context& ctx) {

    }
}