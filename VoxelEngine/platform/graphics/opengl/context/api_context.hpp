#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/resource.hpp>

#include <gl/glew.h>
#include <SDL_opengl.h>
#include <SDL_video.h>


namespace ve::gfx::opengl {
    struct api_context {
        gl_resource<SDL_GLContext> handle;
    };


    extern api_context* get_or_create_context(SDL_Window* window);
    extern api_context* get_context(void);
}