#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/graphics/texture/missing_texture.hpp>


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
            // Because our shaders use a sampler2d[MAX_TEXTURES], not doing this can cause issues,
            // even though we never actually sample from any uninitialized samplers.
            // Note: static so texture lifetime is extended to that of the program.
            static auto tex = texture::create(texture_format_RGBA8, gfx::missing_texture::color_texture.size, 1);
            tex->write(gfx::missing_texture::color_texture);

            std::size_t max_texture_units = (std::size_t) gl_get<i32>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
            for (std::size_t i = 0; i < max_texture_units; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                tex->bind();
            }

            has_context = true;
            return result;
        }();

        return &ctx;
    }


    api_context* get_context(void) {
        return get_or_create_context(nullptr, nullptr);
    }
}