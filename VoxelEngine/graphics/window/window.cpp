#include <VoxelEngine/graphics/window/window.hpp>
#include <VoxelEngine/graphics/window/window_registry.hpp>
#include <VoxelEngine/input/input_manager.hpp>

#include <VoxelEngine/platform/platform_include.hpp>
#include VE_GRAPHICS_INCLUDE(window/window_details.hpp)


namespace ve::graphics {
    using winimpl = windetail::platform_window_methods;
    
    
    window::window(const arguments& args) : layerstack(), owner(args.owner) {
        winimpl::pre_window_create(*this);
        
        handle = SDL_CreateWindow(
            args.title,
            args.position.x,
            args.position.y,
            args.size.x,
            args.size.y,
            args.flags | winimpl::get_flags()
        );
        
        VE_ASSERT(handle, "Failed to create window: "s + SDL_GetError());
    
        winimpl::post_window_create(*this, args.vsync_mode);
        
        set_window_mode(args.window_mode);
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
    
        winimpl::pre_window_destroy(*this);
        window_registry::instance().remove_window(this, owner);
        
        SDL_DestroyWindow(handle);
        handle = nullptr;
    
        winimpl::post_window_destroy(*this);
    }
    
    
    void window::draw(void) {
        VE_ASSERT(handle, "Attempt to draw to closed window.");
    
        winimpl::pre_draw(*this);
        layerstack::draw();
        winimpl::post_draw(*this);
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
        winimpl::set_vsync_mode(*this, mode);
    }
    
    
    void window::minimize(void) {
        SDL_MinimizeWindow(handle);
    }
    
    
    void window::maximize(void) {
        SDL_MaximizeWindow(handle);
    }
    
    
    [[nodiscard]] vec2i window::get_canvas_size(void) const {
        vec2i result;
        SDL_GetWindowSize(handle, &result.x, &result.y);
        return result;
    }
    
    
    void window::set_canvas_size(const vec2i& size) {
        SDL_SetWindowSize(handle, size.x, size.y);
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
}