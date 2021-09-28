#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_bind_state.hpp>


namespace ve::gfx::opengl {
    struct render_context {
        shared<class shader> shader;
        uniform_bind_state uniform_state;
    };
}