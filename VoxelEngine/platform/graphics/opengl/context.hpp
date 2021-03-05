#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>


namespace ve::graphics {
    struct context {
        GLuint current_program;
        u32 next_texture_unit;
    };
}