#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::gfx::opengl {
    struct texture_format {
        GLenum components;      // "format"
        GLint  pixel_format;    // "internal_format"

        std::size_t num_channels;
        std::size_t stride;
        std::array<u8, 4> channel_depths;


        ve_eq_comparable(texture_format);
    };


    constexpr inline auto texture_formats = make_array<texture_format>(
        texture_format { GL_RED,             GL_R8,                  1,  8,  { 8,  0,  0,  0  } },
        texture_format { GL_RG,              GL_RG8,                 2,  16, { 8,  8,  0,  0  } },
        texture_format { GL_RGB,             GL_RGB8,                3,  24, { 8,  8,  8,  0  } },
        texture_format { GL_RGBA,            GL_RGBA8,               4,  32, { 8,  8,  8,  8  } },
        texture_format { GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F,  1,  32, { 32, 0,  0,  0  } },
        texture_format { GL_RGBA,            GL_RGBA32F,             4,  32, { 32, 32, 32, 32 } }
    );

    inline const auto& texture_format_RGBA8    = texture_formats[3];
    inline const auto& texture_format_DEPTH32F = texture_formats[4];
    inline const auto& texture_format_RGBA32F  = texture_formats[5];
}