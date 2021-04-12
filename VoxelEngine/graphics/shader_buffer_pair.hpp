#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/platform_include.hpp>
#include VE_GRAPHICS_INCLUDE(buffer/buffer.hpp)
#include VE_GRAPHICS_INCLUDE(shader/shader_program.hpp)


namespace ve::graphics {
    struct shader_buffer_pair {
        shared<graphics::shader_program> shader;
        shared<graphics::buffer> buffer;
    };
}