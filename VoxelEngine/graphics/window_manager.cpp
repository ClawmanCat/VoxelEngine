#include <VoxelEngine/graphics/window_manager.hpp>
#include <VoxelEngine/graphics/gl_error_callback.hpp>
#include <VoxelEngine/utils/fatal.hpp>
#include <VoxelEngine/utils/logger.hpp>

#include <GL/glew.h>


namespace ve {
    [[nodiscard]] window_manager& window_manager::instance(void) noexcept {
        static window_manager i { };
        return i;
    }
    
    
    void window_manager::create(const char* title, vec2i size, vec2i position, bool maximized, window_mode window_mode, detail::wflags_t flags) {
        std::lock_guard lock { mtx };
        
        handle = SDL_CreateWindow(title, position.x, position.y, size.x, size.y, flags);
        if (!handle) on_fatal_error("Failed to create window.");
        
        if (flags & SDL_WINDOW_OPENGL) {
            // If this is an OpenGL window, create a context and initialize Glew.
            gl_context = SDL_GL_CreateContext(handle);
            if (!gl_context) on_fatal_error("Failed to create OpenGL context.");
            
            GLenum glew_status = glewInit();
            if (glew_status != GLEW_OK) on_fatal_error("Failed to initialize Glew.");
            
            
            SDL_GL_SetSwapInterval(0);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
            
            glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
            
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            
            VE_DEBUG_ONLY(
                glEnable(GL_DEBUG_OUTPUT);
                glDebugMessageCallback(gl_error_callback, nullptr);
            );
        }
        
        set_window_mode(window_mode);
        if (maximized) maximize();
    }
    
    
    window_manager::~window_manager(void) {
        std::lock_guard { mtx };
        
        if (handle) SDL_DestroyWindow(handle);
    }
    
    
    void window_manager::on_frame_start(void) {
        std::lock_guard { mtx };
        
        if (gl_context) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            const auto size = get_canvas_size();
            glViewport(0, 0, size.x, size.y);
        }
    }
    
    
    void window_manager::on_frame_end(void) {
        std::lock_guard { mtx };
        
        if (gl_context) {
            SDL_GL_SwapWindow(handle);
        }
    }
    
    
    void window_manager::set_window_mode(window_mode m) {
        std::lock_guard { mtx };
        
        switch (m) {
            case window_mode::BORDERED:
                SDL_SetWindowBordered(handle, SDL_TRUE);
                SDL_SetWindowFullscreen(handle, 0);
                
                break;
            case window_mode::BORDERLESS:
                SDL_SetWindowFullscreen(handle, SDL_WINDOW_FULLSCREEN_DESKTOP);
                
                break;
            case window_mode::FULLSCREEN:
                SDL_SetWindowFullscreen(handle, SDL_WINDOW_FULLSCREEN);
                
                break;
        }
    }
    
    
    void window_manager::set_vsync_mode(vsync_mode m) {
        std::lock_guard { mtx };
        VE_ASSERT(gl_context);
        
        switch (m) {
            case vsync_mode::IMMEDIATE:
                SDL_GL_SetSwapInterval(0);
                
                break;
            case vsync_mode::VSYNC:
                SDL_GL_SetSwapInterval(1);
                
                break;
            case vsync_mode::ADAPTIVE_VSYNC:
                VE_ASSERT(supports_adaptive_vsync());
                SDL_GL_SetSwapInterval(-1);
                
                break;
        }
    }
    
    
    void window_manager::maximize(void) { std::lock_guard { mtx }; SDL_MaximizeWindow(handle); }
    void window_manager::minimize(void) { std::lock_guard { mtx }; SDL_MinimizeWindow(handle); }
    
    
    bool window_manager::supports_adaptive_vsync(void) {
        VE_ASSERT(gl_context);
        return glewIsSupported("GL_EXT_swap_control_tear");
    }
    
    
    [[nodiscard]] vec2i window_manager::get_window_size(void) const {
        vec2i result;
        SDL_GetWindowSize(handle, &result.x, &result.y);
        return result;
    }
    
    
    void window_manager::set_window_size(const vec2i& size) {
        SDL_SetWindowSize(handle, size.x, size.y);
    }
    
    
    [[nodiscard]] vec2i window_manager::get_canvas_size(void) const {
        vec2i result;
        SDL_GL_GetDrawableSize(handle, &result.x, &result.y);
        return result;
    }
    
    
    void window_manager::set_canvas_size(const vec2i& size) {
        const auto border = get_window_size() - get_canvas_size();
        set_window_size(size + border);
    }
    
    
    [[nodiscard]] window_manager::window_position window_manager::get_window_position(void) const {
        window_position result;
        
        SDL_GetWindowPosition(handle, &result.position.x, &result.position.y);
        result.display = SDL_GetWindowDisplayIndex(handle);
        
        return result;
    }
    
    
    void window_manager::set_window_position(const vec2i& pos) {
        SDL_SetWindowPosition(handle, pos.x, pos.y);
    }
}