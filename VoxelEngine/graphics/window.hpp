#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/layerstack.hpp>

#include <SDL_video.h>
#include <glm/glm.hpp>

#include <optional>
#include <type_traits>
#include <mutex>


namespace ve {
    namespace detail {
        using wflags_t = std::underlying_type_t<SDL_WindowFlags>;
    }
    
    
    // To construct a window, use window_manager::create_window.
    class window : public layerstack {
    public:
        struct window_position {
            vec2i position;
            u32 display;
        };
        
        
        constexpr static inline i32 WINDOW_CENTERED = SDL_WINDOWPOS_CENTERED;
        enum class window_mode { BORDERED, BORDERLESS, FULLSCREEN };
        enum class vsync_mode  { IMMEDIATE, VSYNC, ADAPTIVE_VSYNC };
    
    
        struct arguments {
            const char* title;
            vec2i size              = { 500, 500 };
            vec2i position          = { WINDOW_CENTERED, WINDOW_CENTERED };
            bool maximized          = true;
            window_mode window_mode = window_mode::BORDERED;
            bool handle_close_event = true;
            detail::wflags_t flags  = SDL_WINDOW_RESIZABLE;
        };
        
        
        window(const window&) = delete;
        window& operator=(const window&) = delete;
        
        window(window&&);
        window& operator=(window&&);
    
        ~window(void);
        
        
        void draw(void);
        
        void set_window_mode(window_mode m);
        void set_vsync_mode(vsync_mode m);
        void maximize(void);
        void minimize(void);
        void close(void);
        
        bool supports_adaptive_vsync(void);
        
        
        [[nodiscard]] vec2i get_window_size(void) const;
        void set_window_size(const vec2i& size);
        
        [[nodiscard]] vec2i get_canvas_size(void) const;
        void set_canvas_size(const vec2i& size);
        
        [[nodiscard]] window_position get_window_position(void) const;
        void set_window_position(const vec2i& pos);
        
        [[nodiscard]] u32 get_id(void) const { return SDL_GetWindowID(handle); }
        [[nodiscard]] std::string_view get_name(void) const { return SDL_GetWindowTitle(handle); }
    private:
        SDL_Window* handle = nullptr;
        std::optional<u64> closed_handler_id = std::nullopt;
        
        std::mutex mtx;
        
        
        friend class window_manager;
        window(const arguments& args);
    };
}