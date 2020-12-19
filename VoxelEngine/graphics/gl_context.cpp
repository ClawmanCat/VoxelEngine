#include <VoxelEngine/graphics/gl_context.hpp>
#include <VoxelEngine/graphics/gl_error_callback.hpp>
#include <VoxelEngine/utils/fatal.hpp>


namespace ve::detail {
    struct gl_context_raii_wrapper {
        gl_context_raii_wrapper(SDL_Window* window) {
            ctx = SDL_GL_CreateContext(window);
            if (!ctx) on_fatal_error("Failed to create OpenGL context: "s + SDL_GetError());
            
            // GLEW can only be initialized after an OpenGL context is created.
            GLenum glew_status = glewInit();
            if (glew_status != GLEW_OK) on_fatal_error("Failed to initialize Glew.");
    
            VE_DEBUG_ONLY(
                glEnable(GL_DEBUG_OUTPUT);
                glDebugMessageCallback(gl_error_callback, nullptr);
            );
    
            SDL_GL_SetSwapInterval(0);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
            
            
            // TODO: Some of these settings should not always be enabled. (e.g. GL_DEPTH_TEST for 2D rendering.)
            glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
    
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
        }
        
        
        ~gl_context_raii_wrapper(void) {
            if (ctx) SDL_GL_DeleteContext(ctx);
        }
        
        
        gl_context ctx;
    };
    
    
    gl_context bind_opengl_context(SDL_Window* window) {
        static gl_context_raii_wrapper context { window };
        static SDL_Window* owner = nullptr;
        
        if (window != owner) {
            auto result = SDL_GL_MakeCurrent(window, context.ctx);
            if (result != 0) on_fatal_error("Failed to bind OpenGL context to SDL Window: "s + SDL_GetError());
            
            owner = window;
        }
        
        return context.ctx;
    }
}