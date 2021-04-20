#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/input/keyboard.hpp>
#include <VoxelEngine/input/mouse.hpp>

#include <VoxelEngine/platform/platform_include.hpp>
#include <VoxelEngine/graphics/window/window.hpp>
#include <VoxelEngine/graphics/window/window_registry.hpp>


#define VE_IMPL_INPUT_EVENT(name, ...)              \
struct name : public input_event {                  \
    __VA_ARGS__                                     \
};


namespace ve {
    struct input_event : public event {
        graphics::window* window;
    };
    
    struct input_processing_begin_event : public event {};
    struct input_processing_end_event : public event {};
    struct last_window_closed_event : public event {};
    
    
    
    // Triggers when the key is first pressed down.
    VE_IMPL_INPUT_EVENT(
        key_down_event,
        key_state old_state, new_state;
    );
    
    // Triggers when the key stops being pressed down.
    VE_IMPL_INPUT_EVENT(
        key_up_event,
        key_state old_state, new_state;
    );
    
    // Triggers every tick while the key is held down.
    VE_IMPL_INPUT_EVENT(
        key_hold_event,
        key_state state;
    );
    
    // Triggers in a way that mimics the behaviour of text editors:
    // one time when the key is first pressed down, and then repeatedly a while later if it remains held.
    VE_IMPL_INPUT_EVENT(
        key_typed_event,
        key_state state;
    );
    
    
    
    // Triggers when a mouse button is first pressed down.
    VE_IMPL_INPUT_EVENT(
        mouse_down_event,
        mouse_state old_state, new_state;
        mouse_button button;
    );
    
    // Triggers when a mouse button stops being pressed down.
    VE_IMPL_INPUT_EVENT(
        mouse_up_event,
        mouse_state old_state, new_state;
        mouse_button button;
    );
    
    // Triggers every tick while the mouse button is held down.
    VE_IMPL_INPUT_EVENT(
        mouse_hold_event,
        mouse_state state;
        mouse_button button;
    );
    
    // Triggers every tick while the mouse is moving.
    // begin_state = the state when the mouse started moving.
    // old_state   = the state last tick.
    // new_state   = the state this tick.
    VE_IMPL_INPUT_EVENT(
        mouse_moved_event,
        mouse_state begin_state, old_state, new_state;
    );
    
    // Triggers when the mouse first starts moving.
    // old_state   = the state last tick.
    // new_state   = the state this tick.
    VE_IMPL_INPUT_EVENT(
        mouse_move_start_event,
        mouse_state old_state, new_state;
    );
    
    // Triggers when the mouse stops moving.
    // begin_state = the state when the mouse started moving.
    // old_state   = the state last tick.
    // new_state   = the state this tick.
    VE_IMPL_INPUT_EVENT(
        mouse_move_end_event,
        mouse_state begin_state, old_state, new_state;
    );
    
    // Triggers every tick while the mousewheel is moving.
    // begin_state = the state when the mousewheel started moving.
    // old_state   = the state last tick.
    // new_state   = the state this tick.
    VE_IMPL_INPUT_EVENT(
        mousewheel_moved_event,
        mouse_state begin_state, old_state, new_state;
    );
    
    // Triggers when the mousewheel first starts moving.
    // old_state   = the state last tick.
    // new_state   = the state this tick.
    VE_IMPL_INPUT_EVENT(
        mousewheel_move_start_event,
        mouse_state old_state, new_state;
    );
    
    // Triggers when the mousewheel stops moving.
    // begin_state = the state when the mousewheel started moving.
    // old_state   = the state last tick.
    // new_state   = the state this tick.
    VE_IMPL_INPUT_EVENT(
        mousewheel_move_end_event,
        mouse_state begin_state, old_state, new_state;
    );
    
    
    
    // Wrappers for SDL WindowEvents. See SDL documentation for details.
    VE_IMPL_INPUT_EVENT(window_closed_event   );
    VE_IMPL_INPUT_EVENT(window_maximized_event);
    VE_IMPL_INPUT_EVENT(window_minimized_event);
    VE_IMPL_INPUT_EVENT(window_restored_event );
    VE_IMPL_INPUT_EVENT(window_hidden_event   );
    VE_IMPL_INPUT_EVENT(window_shown_event    );
    VE_IMPL_INPUT_EVENT(window_exposed_event  );
    
    VE_IMPL_INPUT_EVENT(window_gain_keyboard_focus_event);
    VE_IMPL_INPUT_EVENT(window_lose_keyboard_focus_event);
    VE_IMPL_INPUT_EVENT(window_gain_mouse_focus_event   );
    VE_IMPL_INPUT_EVENT(window_lose_mouse_focus_event   );
    
    VE_IMPL_INPUT_EVENT(
        window_resize_event,
        vec2ui old_size, new_size;
    );
    
    VE_IMPL_INPUT_EVENT(
        window_move_event,
        vec2ui old_position, new_position;
    );
}