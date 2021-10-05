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
            vec2ui size                 = vec2ui { 512, 512 };
            window_location position    = { window::WINDOW_CENTERED, window::WINDOW_CENTERED, 0 };
            window_mode window_mode     = window::BORDERED;
            bool start_maximized        = true;
            bool graphics_window        = true;
            sdl_windowflags_t flags     = SDL_WINDOW_RESIZABLE;
            present_mode_t present_mode = present_mode::VSYNC;

            // Note: these are only used when the first graphics window is created.
            const gfxapi::api_settings* api_settings = &gfxapi::default_api_settings;
        };


        ve_shared_only(window, const arguments& args) : std::enable_shared_from_this<window>() {
            if (args.graphics_window) gfxapi::prepare_api_state(args.api_settings);


            handle = SDL_CreateWindow(
                args.title.c_str(),
                args.position.x, args.position.y,
                (i32) args.size.x, (i32) args.size.y,
                args.flags | (args.graphics_window ? gfxapi::window_helpers::get_window_flags() : 0)
            );

            if (args.start_maximized) maximize();


            if (args.graphics_window) {
                gfxapi::get_or_create_context(handle, args.api_settings);
                canvas = make_shared<gfxapi::canvas>(handle, args.present_mode);
            }


            window_registry::instance().add_window(this);
        }


        ~window(void) {
            window_registry::instance().remove_window(this);
            if (handle) SDL_DestroyWindow(handle);
        }


        ve_swap_move_only(window, handle, canvas);


        void begin_frame(void) {
            if (canvas) canvas->begin_frame();
        }


        void end_frame(void) {
            if (canvas) canvas->end_frame();
        }


        void minimize(void) { SDL_MinimizeWindow(handle); }
        void maximize(void) { SDL_MaximizeWindow(handle); }
        void restore (void) { SDL_RestoreWindow (handle); }

        bool is_minimized(void) const { return SDL_GetWindowFlags(handle) & SDL_WINDOW_MINIMIZED; }
        bool is_maximized(void) const { return SDL_GetWindowFlags(handle) & SDL_WINDOW_MAXIMIZED; }


        void set_window_mode(window_mode mode) {
            switch (mode) {
                case window_mode::BORDERED:
                    SDL_SetWindowFullscreen(handle, 0);
                    SDL_SetWindowBordered(handle, SDL_TRUE);
                    break;
                case window_mode::BORDERLESS:
                    SDL_SetWindowFullscreen(handle, 0);
                    SDL_SetWindowBordered(handle, SDL_FALSE);
                    break;
                case window_mode::FULLSCREEN:
                    SDL_SetWindowFullscreen(handle, SDL_WINDOW_FULLSCREEN);
                    break;
            }
        }


        window_mode get_window_mode(void) const {
            auto flags = SDL_GetWindowFlags(handle);

            return flags & SDL_WINDOW_FULLSCREEN
                ? window_mode::FULLSCREEN
                : (flags & SDL_WINDOW_BORDERLESS ? window_mode::BORDERLESS : window_mode::BORDERED);
        }


        vec2ui get_window_size(void) const {
            vec2i result;
            SDL_GetWindowSize(handle, &result.x, &result.y);
            return (vec2ui) result;
        }


        void set_window_size(const vec2ui& size) {
            SDL_SetWindowSize(handle, (i32) size.x, (i32) size.y);
        }


        vec2ui get_canvas_size(void) const {
            return gfxapi::window_helpers::get_canvas_size(handle);
        }


        void set_canvas_size(const vec2ui& size) {
            vec2ui border = get_window_size() - get_canvas_size();
            set_window_size(size + border);
        }


        present_mode_t get_present_mode(void) const {
            return canvas->get_mode();
        }


        void set_present_mode(present_mode_t mode) {
            canvas->set_present_mode(mode);
        }


        VE_GET_CREF(handle);
        VE_GET_CREF(canvas);
    private:
        SDL_Window* handle = nullptr;
        shared<gfxapi::canvas> canvas = nullptr;
    };
}