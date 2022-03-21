#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::gfx::opengl {
    struct texture_format {
        std::string_view name;

        GLenum components;      // Component channels (RGB / RGBA / Depth / Etc.)
        GLenum component_type;  // Type of each channel (Float / UByte / Etc.)
        GLint  pixel_format;    // Pixel format (RGBA8 / RGBA32F / DEPTH32F / Etc.)

        std::size_t num_channels;
        std::size_t stride;
        std::array<u8, 4> channel_depths;

        std::size_t alpha_channel = 3;


        ve_eq_comparable(texture_format);


        bool is_floating_point_texture(void) const { return component_type == GL_FLOAT || component_type == GL_DOUBLE; }
        bool is_integer_texture(void) const { return !is_floating_point_texture(); }
        bool is_scalar_texture(void) const { return num_channels == 1; }
        bool has_alpha_channel(void) const { return channel_depths[alpha_channel] != 0; }
    };


    constexpr inline auto texture_formats = make_array<texture_format>(
        texture_format { "R8",       GL_RED,             GL_UNSIGNED_BYTE, GL_R8,                  1,  8,  { 8,  0,  0,  0  } },
        texture_format { "RG8",      GL_RG,              GL_UNSIGNED_BYTE, GL_RG8,                 2,  16, { 8,  8,  0,  0  } },
        texture_format { "RGB8",     GL_RGB,             GL_UNSIGNED_BYTE, GL_RGB8,                3,  24, { 8,  8,  8,  0  } },
        texture_format { "RGBA8",    GL_RGBA,            GL_UNSIGNED_BYTE, GL_RGBA8,               4,  32, { 8,  8,  8,  8  } },
        texture_format { "DEPTH32F", GL_DEPTH_COMPONENT, GL_FLOAT,         GL_DEPTH_COMPONENT32F,  1,  32, { 32, 0,  0,  0  } },
        texture_format { "RGBA32F",  GL_RGBA,            GL_FLOAT,         GL_RGBA32F,             4,  32, { 32, 32, 32, 32 } }
    );


    // Names for commonly used formats.
    constexpr inline const auto& texture_format_RGB8     = texture_formats[2];
    constexpr inline const auto& texture_format_RGBA8    = texture_formats[3];
    constexpr inline const auto& texture_format_DEPTH32F = texture_formats[4];
    constexpr inline const auto& texture_format_RGBA32F  = texture_formats[5];
}