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

        // For default formats, just find the first format that has the required components.
        texture_format color_buffer_format = *ranges::find_if(texture_formats, equal_on(&texture_format::components, (GLenum) GL_RGBA));
        texture_format depth_buffer_format = *ranges::find_if(texture_formats, equal_on(&texture_format::components, (GLenum) GL_DEPTH_COMPONENT));

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