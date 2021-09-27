#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx::vulkan {
    class render_target;
    class command_buffer;

    struct render_context {
        render_target* target;
        command_buffer* cmd_buffer;
    };
}