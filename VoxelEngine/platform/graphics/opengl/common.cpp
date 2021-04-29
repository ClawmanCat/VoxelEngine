#include <VoxelEngine/platform/graphics/opengl/common.hpp>
#include <VoxelEngine/platform/graphics/opengl/error_callback.hpp>


namespace ve::graphics {
    namespace detail {
        // Singleton RAII object that manages setting up OpenGL when it is created and manages cleanup when it is destroyed.
        // The instance for this class is constructed as a Meyers Singleton in bind_opengl_context.
        struct opengl_context_raii_wrapper {
            opengl_context ctx;
            
            
            opengl_context_raii_wrapper(SDL_Window* window) {
                // Create OpenGL context and bind it to the window.
                ctx = SDL_GL_CreateContext(window);
                VE_ASSERT(ctx, "Failed to create OpenGL context: "s + SDL_GetError() + ".");
                
                // Initialize GLEW. (Must be done after context creation.)
                GLenum glew_status = glewInit();
                
                VE_ASSERT(
                    glew_status == GLEW_OK,
                    "Failed to initialize Glew: "s + (const char*) glewGetErrorString(glew_status) + "."
                );
                
                // Log OpenGL errors in debug mode.
                VE_DEBUG_ONLY(
                    glEnable(GL_DEBUG_OUTPUT);
                    glDebugMessageCallback(opengl_error_callback, nullptr);
                );
                
                
                glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
    
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_DEPTH_CLAMP);
            }
            
            
            ~opengl_context_raii_wrapper(void) {
                if (ctx) SDL_GL_DeleteContext(ctx);
            }
        };
    }
    
    
    opengl_context bind_opengl_context(SDL_Window* window) {
        static detail::opengl_context_raii_wrapper context_owner { window };
        static SDL_Window* current_owner = nullptr;
        
        if (current_owner != window) {
            auto result = SDL_GL_MakeCurrent(window, context_owner.ctx);
            VE_ASSERT(result == 0, "'Failed to bind OpenGL context to window: "s + SDL_GetError() + ".");
            
            current_owner = window;
        }
        
        return context_owner.ctx;
    }
    
    
    #define GL_ERROR_CASE(name) if (error == name) return #name
    
    std::string_view get_gl_error_string(GLenum error) {
        GL_ERROR_CASE(GL_INVALID_ENUM);
        GL_ERROR_CASE(GL_INVALID_VALUE);
        GL_ERROR_CASE(GL_INVALID_OPERATION);
        GL_ERROR_CASE(GL_STACK_OVERFLOW);
        GL_ERROR_CASE(GL_STACK_UNDERFLOW);
        GL_ERROR_CASE(GL_OUT_OF_MEMORY);
        GL_ERROR_CASE(GL_NO_ERROR);
        
        return "Unknown Error";
    }
}