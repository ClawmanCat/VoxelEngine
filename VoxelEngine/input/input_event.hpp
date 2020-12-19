#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/input/key_state.hpp>
#include <VoxelEngine/input/key_mods.hpp>
#include <VoxelEngine/input/button_state.hpp>
#include <VoxelEngine/input/mouse_state.hpp>


#define VE_IMPL_EMPTY_INPUT_EVENT(name)                 \
struct name : public input_event {                      \
    name(u32 window_id) : input_event(window_id) {}     \
};


namespace ve {
    struct input_event : public events::event {
        u32 window_id;
        
        input_event(void) = default;
        input_event(u32 window_id) : events::event(), window_id(window_id) {}
    };
    
    
    struct pre_input_processed_event : public events::event {};
    struct post_input_processed_event : public events::event {};
    
    
    // Keyboard Events
    template <bool is_down> struct key_press_event : public input_event {
        key_state old_state, new_state;
    };
    
    using key_down_event = key_press_event<true>;
    using key_up_event   = key_press_event<false>;
    
    
    struct key_hold_event : public input_event {
        key_state state;
    };
    
    // Like key_hold_event but only triggers at set intervals, like in a text editor.
    struct key_typed_event : public input_event {
        key_state state;
    };
    
    
    // Mouse Events
    template <bool is_down> struct mouse_press_event : public input_event {
        button_state old_state, new_state;
    };
    
    using mouse_down_event = mouse_press_event<true>;
    using mouse_up_event   = mouse_press_event<false>;
    
    
    struct mouse_hold_event : public input_event {
        button_state state;
    };
    
    
    struct mouse_moved_event : public input_event {
        mouse_state prev_state, current_state;
    };
    
    struct mouse_move_start_event : public input_event {
        mouse_state prev_state, current_state;
    };
    
    struct mouse_move_end_event : public input_event {
        mouse_state begin_state, end_state;
        u64 ticks_elapsed;
    };
    
    
    struct mousewheel_moved_event : public input_event {
        mouse_state prev_state, current_state;
    };
    
    struct mousewheel_move_start_event : public input_event {
        mouse_state prev_state, current_state;
    };
    
    struct mousewheel_move_end_event : public input_event {
        mouse_state begin_state, end_state;
        u64 ticks_elapsed;
    };
    
    
    VE_IMPL_EMPTY_INPUT_EVENT(window_closed_event   );
    VE_IMPL_EMPTY_INPUT_EVENT(window_maximized_event);
    VE_IMPL_EMPTY_INPUT_EVENT(window_minimized_event);
    VE_IMPL_EMPTY_INPUT_EVENT(window_restored_event );
    VE_IMPL_EMPTY_INPUT_EVENT(window_hidden_event   );
    VE_IMPL_EMPTY_INPUT_EVENT(window_shown_event    );
    VE_IMPL_EMPTY_INPUT_EVENT(window_exposed_event  );
    
    
    VE_IMPL_EMPTY_INPUT_EVENT(window_gain_keyboard_focus_event);
    VE_IMPL_EMPTY_INPUT_EVENT(window_lose_keyboard_focus_event);
    VE_IMPL_EMPTY_INPUT_EVENT(window_gain_mouse_focus_event   );
    VE_IMPL_EMPTY_INPUT_EVENT(window_lose_mouse_focus_event   );
    
    
    struct window_resized_event : public input_event {
        window_resized_event(u32 window_id, const vec2i& old_size, const vec2i& new_size)
            : input_event(window_id), old_size(old_size), new_size(new_size) {}
        
        vec2i old_size, new_size;
    };
    
    
    struct window_moved_event : public input_event {
        window_moved_event(u32 window_id, const vec2i& old_pos, const vec2i& new_pos, u32 old_display, u32 new_display)
            : input_event(window_id), old_pos(old_pos), new_pos(new_pos), old_display(old_display), new_display(new_display)
        {}
        
        vec2i old_pos, new_pos;
        u32 old_display, new_display;
    };
}