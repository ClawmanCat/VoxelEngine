#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/reset_texture_bindings.hpp>
#include <VoxelEngine/utility/assert.hpp>


namespace ve::gfx::opengl {
    bool has_context = false;


    void prepare_api_state(const api_settings* settings) {
        if (has_context) return; // This method must be called before window creation.


        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

        if (settings->logging_enabled) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        }
    }


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

            // Set up logger.
            if (settings->logging_enabled) {
                register_opengl_logger(settings->logging_callback);
            }

            // Pre-initialize each texture unit with an actual texture.
            // Shaders used by the engine may declare more sample uniforms than are actually used
            // (e.g. requiring a sampler array of constant size and reading the index from the vertices.),
            // which raises UB warnings if no value is bound to these samplers.
            reset_texture_bindings();

            has_context = true;
            return result;
        }();

        return &ctx;
    }


    api_context* get_context(void) {
        return get_or_create_context(nullptr, nullptr);
    }
}