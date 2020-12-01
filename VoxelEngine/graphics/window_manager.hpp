#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/immovable.hpp>

#include <SDL_video.h>
#include <glm/glm.hpp>

#include <type_traits>
#include <mutex>


namespace ve {
    namespace detail {
        using wflags_t = std::underlying_type_t<SDL_WindowFlags>;
    }
    
    
    class window_manager {
    public:
        struct window_position {
            vec2i position;
            u32 display;
        };
        
        
        constexpr static inline i32 WINDOW_CENTERED = SDL_WINDOWPOS_CENTERED;
        enum class window_mode { BORDERED, BORDERLESS, FULLSCREEN };
        enum class vsync_mode  { IMMEDIATE, VSYNC, ADAPTIVE_VSYNC };
        
        
        [[nodiscard]] static window_manager& instance(void) noexcept;
        
        // TODO: Auto create when the engine is initialized.
        void create(
            const char* title,
            vec2i size              = { 500, 500 },
            vec2i position          = { WINDOW_CENTERED, WINDOW_CENTERED },
            bool maximized          = true,
            window_mode window_mode = window_mode::BORDERED,
            detail::wflags_t flags  = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );
        
        ~window_manager(void);
        
        
        void on_frame_start(void);
        void on_frame_end(void);
        
        void set_window_mode(window_mode m);
        void set_vsync_mode(vsync_mode m);
        void maximize(void);
        void minimize(void);
        
        bool supports_adaptive_vsync(void);
        
        
        [[nodiscard]] vec2i get_window_size(void) const;
        void set_window_size(const vec2i& size);
        
        [[nodiscard]] vec2i get_canvas_size(void) const;
        void set_canvas_size(const vec2i& size);
        
        [[nodiscard]] window_position get_window_position(void) const;
        void set_window_position(const vec2i& pos);
    private:
        window_manager(void) = default;
        ve_make_immovable;
        
        SDL_Window* handle = nullptr;
        SDL_GLContext gl_context = nullptr;
        std::recursive_mutex mtx;
    };
}