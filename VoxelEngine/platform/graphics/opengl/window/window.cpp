#include <VoxelEngine/platform/graphics/opengl/window/window.hpp>
#include <VoxelEngine/platform/graphics/opengl/window/window_registry.hpp>
#include <VoxelEngine/platform/graphics/opengl/common.hpp>
#include <VoxelEngine/input/input_manager.hpp>


namespace ve::graphics {
    window::window(const arguments& args) : layerstack(), owner(args.owner) {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
        
        handle = SDL_CreateWindow(
            args.title,
            args.position.x,
            args.position.y,
            args.size.x,
            args.size.y,
            args.flags | SDL_WINDOW_OPENGL
        );
        
        VE_ASSERT(handle, "Failed to create window: "s + SDL_GetError());
        
        set_window_mode(args.window_mode);
        set_vsync_mode(args.vsync_mode);
        if (args.maximized) maximize();
        
        window_registry::instance().add_window(this, args.owner);
        
        // Close window if close button is pressed.
        input_manager::instance()
            .get_dispatcher()
            .add_handler<window_closed_event>(
                (event_handler) [this](const event& e) {
                    auto evnt = (const window_closed_event&) e;
                    if (evnt.window == this) close();
                }
            );
    }
    
    
    window::~window(void) {
        close();
    }
    
    
    void window::close(void) {
        if (!handle) return;
    
        window_registry::instance().remove_window(this, owner);
        
        SDL_DestroyWindow(handle);
        handle = nullptr;
    }
    
    
    void window::draw(void) {
        VE_ASSERT(handle, "Attempt to draw to closed window.");
        bind_opengl_context(handle);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        const auto size = get_canvas_size();
        glViewport(0, 0, size.x, size.y);
        
        layerstack::draw();
        
        SDL_GL_SwapWindow(handle);
    }
    
    
    void window::set_window_mode(window_mode mode) {
        switch (mode) {
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
    
    
    void window::set_vsync_mode(vsync_mode mode) {
        bind_opengl_context(handle);
        
        switch (mode) {
            case vsync_mode::IMMEDIATE:
                SDL_GL_SetSwapInterval(0);
                
                break;
            case vsync_mode::VSYNC:
                SDL_GL_SetSwapInterval(1);
                
                break;
            case vsync_mode::ADAPTIVE_VSYNC:
                VE_ASSERT(supports_adaptive_vsync(), "Adaptive Vsync is not supported on this device.");
                SDL_GL_SetSwapInterval(-1);
                
                break;
        }
    }
    
    
    void window::minimize(void) {
        SDL_MinimizeWindow(handle);
    }
    
    
    void window::maximize(void) {
        SDL_MaximizeWindow(handle);
    }
    
    
    [[nodiscard]] vec2i window::get_window_size(void) const {
        vec2i result;
        SDL_GetWindowSize(handle, &result.x, &result.y);
        return result;
    }
    
    
    void window::set_window_size(const vec2i& size) {
        SDL_SetWindowSize(handle, size.x, size.y);
    }
    
    
    [[nodiscard]] vec2i window::get_canvas_size(void) const {
        bind_opengl_context(handle);
    
        vec2i result;
        SDL_GL_GetDrawableSize(handle, &result.x, &result.y);
        
        return result;
    }
    
    
    void window::set_canvas_size(const vec2i& size) {
        const auto border = get_window_size() - get_canvas_size();
        set_window_size(size + border);
    }
    
    
    [[nodiscard]] window::window_location window::get_window_location(void) const {
        window_location result;
        
        SDL_GetWindowPosition(handle, &result.position.x, &result.position.y);
        result.display = SDL_GetWindowDisplayIndex(handle);
        
        return result;
    }
    
    
    void window::set_window_location(const vec2i& pos) {
        SDL_SetWindowPosition(handle, pos.x, pos.y);
    }
    
    
    std::string window::get_window_title(void) const {
        return std::string { SDL_GetWindowTitle(handle) };
    }
    
    
    void window::set_window_title(const char* title) {
        SDL_SetWindowTitle(handle, title);
    }
    
    
    u32 window::get_window_id(void) const {
        return SDL_GetWindowID(handle);
    }
    
    
    bool window::supports_adaptive_vsync(void) {
        return glewIsSupported("GL_EXT_swap_control_tear");
    }
}