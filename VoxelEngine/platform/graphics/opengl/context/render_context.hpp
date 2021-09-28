#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>


namespace ve::gfx::opengl {
    struct render_context {
        shared<class shader> shader;
    };
}