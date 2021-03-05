#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>
#include <SDL.h>

#include <string_view>


namespace ve::graphics {
    using opengl_context = SDL_GLContext;
    
    // Binds the OpenGL context to the window, creating it if necessary.
    extern opengl_context bind_opengl_context(SDL_Window* window);
    
    // Converts an OpenGL error to a string.
    extern std::string_view get_gl_error_string(GLenum error);
}