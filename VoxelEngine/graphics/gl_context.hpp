#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>
#include <SDL.h>


namespace ve::detail {
    using gl_context = SDL_GLContext;
    
    // Creates an OpenGL context if it doesn't exist, and binds it to the given window.
    extern gl_context bind_opengl_context(SDL_Window* window);
}