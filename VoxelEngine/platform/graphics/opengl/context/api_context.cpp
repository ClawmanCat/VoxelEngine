#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/utility/assert.hpp>


namespace ve::gfx::opengl {
    api_context* get_or_create_context(SDL_Window* window, const api_settings* settings) {
        static api_context ctx = [&] {
            VE_ASSERT(window, "Please provide a window when creating the OpenGL context.");

            // Create context.
            api_context result = {
                .handle   = gl_resource<SDL_GLContext> { SDL_GL_CreateContext(window), SDL_GL_DeleteContext },
                .settings = *settings
            };

            // Initialize Glew.
            if (auto glew_status = glewInit(); glew_status != GLEW_OK) {
                VE_ASSERT(false, "Failed to initialize Glew: ", (const char*) glewGetErrorString(glew_status));
            }

            return result;
        }();

        return &ctx;
    }


    api_context* get_context(void) {
        return get_or_create_context(nullptr, nullptr);
    }
}