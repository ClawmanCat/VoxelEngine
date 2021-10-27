#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/presentation/present_mode.hpp>
#include <VoxelEngine/graphics/presentation/window_registry.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(presentation/canvas.hpp)
#include VE_GFX_HEADER(presentation/window_helpers.hpp)
#include VE_GFX_HEADER(context/api_context.hpp)

#include <SDL_video.h>


namespace ve::gfx {
    using sdl_windowflags_t = std::underlying_type_t<SDL_WindowFlags>;


    class window : public std::enable_shared_from_this<window> {
    public:
        constexpr static inline i32 WINDOW_CENTERED = SDL_WINDOWPOS_CENTERED;

        enum window_mode { BORDERED, BORDERLESS, FULLSCREEN };
        struct window_location { i32 x, y; u32 display; };


        struct arguments {
            std::string title;
            vec2ui size                    = vec2ui { 512, 512 };
            window_location position       = { window::WINDOW_CENTERED, window::WINDOW_CENTERED, 0 };
            window_mode window_mode        = window::BORDERED;
            bool start_maximized           = true;
            bool graphics_window           = true;
            bool exit_button_closes_window = true;
            sdl_windowflags_t flags        = SDL_WINDOW_RESIZABLE;
            present_mode_t present_mode    = present_mode::VSYNC;

            // Note: these are only used when the first graphics window is created.
            const gfxapi::api_settings* api_settings = &gfxapi::default_api_settings;
        };


        ve_shared_only(window, const arguments& args) : std::enable_shared_from_this<window>() {
            init(args);
        }

        ~window(void);
        ve_swap_move_only(window, handle, canvas);


        void begin_frame(void);
        void end_frame(void);


        void close(void);
        bool is_closed(void) const;

        void minimize(void) { SDL_MinimizeWindow(handle); }
        void maximize(void) { SDL_MaximizeWindow(handle); }
        void restore (void) { SDL_RestoreWindow (handle); }

        bool is_minimized(void) const { return SDL_GetWindowFlags(handle) & SDL_WINDOW_MINIMIZED; }
        bool is_maximized(void) const { return SDL_GetWindowFlags(handle) & SDL_WINDOW_MAXIMIZED; }

        void set_window_mode(window_mode mode);
        window_mode get_window_mode(void) const;

        vec2ui get_window_size(void) const;
        void set_window_size(const vec2ui& size);
        vec2ui get_canvas_size(void) const;
        void set_canvas_size(const vec2ui& size);

        window_location get_location(void) const;
        vec2i get_position(void) const;
        void set_position(const vec2i& position);

        present_mode_t get_present_mode(void) const;
        void set_present_mode(present_mode_t mode);


        VE_GET_CREF(handle);
        VE_GET_CREF(canvas);
    private:
        SDL_Window* handle = nullptr;
        shared<gfxapi::canvas> canvas = nullptr;


        void init(const window::arguments& args);
    };
}