#include <VoxelEngine/graphics/presentation/window_registry.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


namespace ve::gfx {
    window_registry& window_registry::instance(void) {
        static window_registry i;
        return i;
    }


    void window_registry::begin_frame(void) {
        for (auto* window : windows) window->begin_frame();
    }


    void window_registry::end_frame(void) {
        for (auto* window : windows) window->end_frame();
    }


    void window_registry::add_window(window* window) {
        windows.push_back(window);
    }


    void window_registry::remove_window(window* window) {
        // std::erase seems to throw with the MSVC STL if windows is empty. I think this is incorrect behaviour.
        if (auto it = ranges::find(windows, window); it != windows.end()) {
            windows.erase(it);
        }
    }
}