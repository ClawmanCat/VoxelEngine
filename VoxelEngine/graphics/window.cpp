#include <VoxelEngine/graphics/window.hpp>
#include <VoxelEngine/graphics/gl_context.hpp>
#include <VoxelEngine/utils/fatal.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/input/input_event.hpp>
#include <VoxelEngine/graphics/window_manager.hpp>
#include <VoxelEngine/event/handle.hpp>

#include <GL/glew.h>


#define VE_IMPL_CHECK_WINDOW_EXISTS(action, ...)                                                    \
if (!handle) {                                                                                      \
    VE_LOG_ERROR("An attempt was made to "s + action + " a window which has already been closed."); \
    return __VA_ARGS__;                                                                             \
}


namespace ve {
    window::window(const arguments& args) {
        handle = SDL_CreateWindow(args.title, args.position.x, args.position.y, args.size.x, args.size.y, args.flags | SDL_WINDOW_OPENGL);
        if (!handle) on_fatal_error("Failed to create window.");
    
        detail::bind_opengl_context(handle);
        
        if (args.handle_close_event) {
            closed_handler_id = ve_handle_event(
                input_manager::instance().get_dispatcher(),
                window_closed_event,
                (id = get_id()),
                {
                    if (event.window_id == id) {
                        window_manager::instance().get_window(id).lock()->close();
                    }
                }
            );
        }
        
        set_window_mode(args.window_mode);
        if (args.maximized) maximize();
    }
    
    
    window::window(window&& o) {
        *this = std::move(o);
    }
    
    
    window& window::operator=(window&& o) {
        std::scoped_lock lock { mtx, o.mtx };
        std::swap(handle, o.handle);
        
        return *this;
    }
    
    
    
    window::~window(void) {
        close();
    }
    
    
    void window::draw(void) {
        std::lock_guard { mtx };
    
        VE_IMPL_CHECK_WINDOW_EXISTS("draw to");
    
        detail::bind_opengl_context(handle);
    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        const auto size = get_canvas_size();
        glViewport(0, 0, size.x, size.y);
        
        layerstack::draw();
    
        SDL_GL_SwapWindow(handle);
    }
    
    
    void window::set_window_mode(window_mode m) {
        std::lock_guard { mtx };
    
        VE_IMPL_CHECK_WINDOW_EXISTS("modify");
        
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
    
    
    void window::set_vsync_mode(vsync_mode m) {
        std::lock_guard { mtx };
    
        VE_IMPL_CHECK_WINDOW_EXISTS("modify");
        
        detail::bind_opengl_context(handle);
        
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
    
    
    void window::maximize(void) { std::lock_guard {mtx }; VE_IMPL_CHECK_WINDOW_EXISTS("modify"); SDL_MaximizeWindow(handle); }
    void window::minimize(void) { std::lock_guard {mtx }; VE_IMPL_CHECK_WINDOW_EXISTS("modify"); SDL_MinimizeWindow(handle); }
    
    
    void window::close(void) {
        if (closed_handler_id.has_value()) {
            input_manager::instance().get_dispatcher().remove_handler<window_closed_event>(closed_handler_id.value());
            closed_handler_id = std::nullopt;
        }
    
        if (handle) {
            window_manager::instance().remove_window(get_id());
            SDL_DestroyWindow(handle);
            
            handle = nullptr;
        }
    }
    
    
    bool window::supports_adaptive_vsync(void) {
        return glewIsSupported("GL_EXT_swap_control_tear");
    }
    
    
    [[nodiscard]] vec2i window::get_window_size(void) const {
        VE_IMPL_CHECK_WINDOW_EXISTS("get the state of", { 0, 0 });
        
        vec2i result;
        SDL_GetWindowSize(handle, &result.x, &result.y);
        return result;
    }
    
    
    void window::set_window_size(const vec2i& size) {
        VE_IMPL_CHECK_WINDOW_EXISTS("modify");
        SDL_SetWindowSize(handle, size.x, size.y);
    }
    
    
    [[nodiscard]] vec2i window::get_canvas_size(void) const {
        VE_IMPL_CHECK_WINDOW_EXISTS("get the state of", { 0, 0 });
    
        vec2i result;
        
        detail::bind_opengl_context(handle);
        SDL_GL_GetDrawableSize(handle, &result.x, &result.y);
        
        return result;
    }
    
    
    void window::set_canvas_size(const vec2i& size) {
        VE_IMPL_CHECK_WINDOW_EXISTS("modify");
        
        const auto border = get_window_size() - get_canvas_size();
        set_window_size(size + border);
    }
    
    
    [[nodiscard]] window::window_position window::get_window_position(void) const {
        VE_IMPL_CHECK_WINDOW_EXISTS("get the state of", { { 0, 0 }, 0 });
    
        window_position result;
        
        SDL_GetWindowPosition(handle, &result.position.x, &result.position.y);
        result.display = SDL_GetWindowDisplayIndex(handle);
        
        return result;
    }
    
    
    void window::set_window_position(const vec2i& pos) {
        VE_IMPL_CHECK_WINDOW_EXISTS("modify");
        SDL_SetWindowPosition(handle, pos.x, pos.y);
    }
}