#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/resource.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/debug.hpp>

#include <gl/glew.h>
#include <SDL_opengl.h>
#include <SDL_video.h>


namespace ve::gfx::opengl {
    struct api_settings {
        // API logging settings.
        bool logging_enabled = VE_IF_DEBUG_ELSE(true, false);
        decltype(&opengl_logging_callback) logging_callback = opengl_logging_callback;

        // Default formats for window canvas framebuffer.
        texture_format color_buffer_format = texture_format_RGBA8;
        texture_format depth_buffer_format = texture_format_DEPTH32F;

        // Default number of mipmap levels for all textures that do not specify the number of levels explicitly.
        std::size_t num_mipmap_levels = 4;
    };

    const static inline api_settings default_api_settings = api_settings { };


    struct api_context {
        gl_resource<SDL_GLContext> handle;
        api_settings settings;
    };


    extern void         prepare_api_state(const api_settings* settings = &default_api_settings);
    extern api_context* get_or_create_context(SDL_Window* window, const api_settings* settings = &default_api_settings);
    extern api_context* get_context(void);
}