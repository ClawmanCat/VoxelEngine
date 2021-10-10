#include <VoxelEngine/graphics/presentation/window_registry.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/input/window.hpp>
#include <VoxelEngine/utility/functional.hpp>


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

        if (!std::exchange(had_windows, true)) {
            input_manager::instance().trigger_event(first_window_opened_event { window });
        }

        input_manager::instance().trigger_event(window_opened_event { window });
    }


    void window_registry::remove_window(window* window) {
        // std::erase seems to throw with the MSVC STL if windows is empty for some reason.
        if (auto it = ranges::find(windows, window); it != windows.end()) {
            windows.erase(it);

            // Note: don't trigger normal window_closed_event since that one is actually handled by SDL.
            if (windows.empty()) input_manager::instance().trigger_event(last_window_closed_event { window });
        }
    }


    window* window_registry::get_window(SDL_Window* sdl_window) {
        return &*ranges::find_if(windows | views::indirect, equal_on(&window::get_handle, sdl_window));
    }
}