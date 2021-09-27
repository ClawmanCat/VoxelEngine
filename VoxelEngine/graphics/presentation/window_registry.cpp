#include <VoxelEngine/graphics/presentation/window_registry.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


namespace ve::gfx {
    window_registry& window_registry::instance(void) {
        static window_registry i;
        return i;
    }


    void window_registry::begin_frame(void) {
        foreach([&](auto& window) { window.begin_frame(); });
    }


    void window_registry::end_frame(void) {
        foreach([&](auto& window) { window.end_frame(); });
    }


    void window_registry::add_window(shared<window> window) {
        windows.push_back(window);
    }


    void window_registry::remove_window(shared<window> window) {
        std::erase_if(windows, [&] (const weak<class window>& maybe_remove) {
            auto ptr = maybe_remove.lock();
            return !ptr || ptr == window;
        });
    }

}