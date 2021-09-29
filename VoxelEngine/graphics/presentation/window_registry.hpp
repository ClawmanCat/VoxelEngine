#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::gfx {
    class window;


    class window_registry {
    public:
        static window_registry& instance(void);

        void begin_frame(void);
        void end_frame(void);

        void add_window(window* window);
        void remove_window(window* window);
    private:
        std::vector<window*> windows;
    };
}