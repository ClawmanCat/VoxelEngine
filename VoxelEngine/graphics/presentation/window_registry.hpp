#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>

#include <SDL_video.h>


namespace ve::gfx {
    class window;


    class window_registry {
    public:
        static window_registry& instance(void);

        void begin_frame(void);
        void end_frame(void);

        void add_window(window* window);
        void remove_window(window* window);

        window* get_window(SDL_Window* sdl_window);
        VE_GET_CREF(windows);
    private:
        std::vector<window*> windows;
        bool had_windows = false;
    };
}