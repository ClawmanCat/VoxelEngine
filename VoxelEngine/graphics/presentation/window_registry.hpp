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

        void add_window(shared<window> window);
        void remove_window(shared<window> window);
    private:
        std::vector<weak<window>> windows;


        void foreach(auto pred) {
            for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
                if (auto handle = it->lock(); handle) {
                    pred(*handle);
                } else {
                    swap_erase(windows, it.base());
                }
            }
        }
    };
}