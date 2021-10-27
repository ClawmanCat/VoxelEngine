#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/input_event.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


namespace ve {
    struct ve_input_event(window_opened_event) {};
    struct ve_input_event(window_closed_event) {};
    struct ve_input_event(first_window_opened_event) {};
    struct ve_input_event(last_window_closed_event)  {};
    struct exit_requested_event {}; // Note: no associated window!

    struct ve_input_event(window_maximized_event) {};
    struct ve_input_event(window_minimized_event) {};
    struct ve_input_event(window_restored_event)  {};

    struct ve_input_event(window_hidden_event)  {};
    struct ve_input_event(window_shown_event)   {};
    struct ve_input_event(window_exposed_event) {};

    struct ve_input_event(window_gain_keyboard_focus_event) {};
    struct ve_input_event(window_lose_keyboard_focus_event) {};
    struct ve_input_event(window_gain_mouse_focus_event)    {};
    struct ve_input_event(window_lose_mouse_focus_event)    {};

    struct ve_input_event(window_resized_event) { vec2ui old_size, new_size; };
    struct ve_input_event(window_moved_event)   { gfx::window::window_location old_position, new_position; };
}