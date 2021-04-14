#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/window/layerstack.hpp>

#include <SDL.h>

#include <type_traits>


namespace ve::detail::window_details {
    struct platform_window_data;
    struct platform_window_methods;
}

namespace ve::graphics {
    namespace windetail = ve::detail::window_details;
    
    
    namespace detail {
        using window_flags_t = std::underlying_type_t<SDL_WindowFlags>;
    }
    
    
    // The window class contains a platform agnostic API to create and draw to a window.
    // Platform-specific implementation details are handled in {current platform}/window/window_detail.hpp
    class window : public layerstack {
    public:
        constexpr static inline i32 WINDOW_CENTERED = SDL_WINDOWPOS_CENTERED;
        struct window_location { vec2i position; u32 display; };
        enum class window_mode { BORDERED, BORDERLESS, FULLSCREEN };
        enum class vsync_mode  { IMMEDIATE, VSYNC, ADAPTIVE_VSYNC, TRIPLE_BUFFERED };
        
        
        // Use named arguments since the window constructor has a lot of parameters.
        struct arguments {
            const char* title;
            ve_default_actor(owner);
            vec2i size                   = { 500, 500 };
            vec2i position               = { WINDOW_CENTERED, WINDOW_CENTERED };
            bool maximized               = true;
            window_mode window_mode      = window_mode::BORDERED;
            vsync_mode vsync_mode        = vsync_mode::IMMEDIATE;
            detail::window_flags_t flags = SDL_WINDOW_RESIZABLE;
        };
        
        explicit window(const arguments& args);
        ~window(void);
        
        window(const window&) = delete;
        window& operator=(const window&) = delete;
        
        window(window&& other) : layerstack(std::move(other)) {
            std::swap(handle, other.handle);
        }
        
        
        void close(void);
        
        void draw(void);
        
        void set_window_mode(window_mode mode);
        void set_vsync_mode(vsync_mode mode);
        
        void minimize(void);
        void maximize(void);
        
        [[nodiscard]] vec2i get_canvas_size(void) const;
        void set_canvas_size(const vec2i& size);
    
        [[nodiscard]] window_location get_window_location(void) const;
        void set_window_location(const vec2i& pos);
    
        std::string get_window_title(void) const;
        void set_window_title(const char* title);
        
        u32 get_window_id(void) const;
        
        VE_GET_VAL(handle);
        VE_GET_VAL(owner);
    
    private:
        friend struct windetail::platform_window_methods;
        unique<windetail::platform_window_data> platform_data = nullptr;
        
        SDL_Window* handle = nullptr;
        actor_id owner;
    };
}