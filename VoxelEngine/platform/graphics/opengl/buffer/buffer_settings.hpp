#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>


namespace ve::graphics {
    enum class buffer_primitive : GLenum {
        POINTS     = GL_POINTS,
        LINES      = GL_LINES,
        TRIANGLES  = GL_TRIANGLES,
        QUADS      = GL_QUADS
    };
    
    
    enum class buffer_storage_mode : GLenum {
        WRITE_ONCE_READ_ONCE = GL_STREAM_DRAW,
        WRITE_ONCE_READ_MANY = GL_STATIC_DRAW,
        WRITE_MANY_READ_MANY = GL_DYNAMIC_DRAW
    };
}