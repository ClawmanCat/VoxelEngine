#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/delayed_event_dispatcher.hpp>
#include <VoxelEngine/input/keyboard.hpp>
#include <VoxelEngine/input/mouse.hpp>
#include <VoxelEngine/input/input_event.hpp>


#define ve_register_input_event(type, event_name, ...)                  \
ve::input_manager::instance()                                           \
    .get_dispatcher()                                                   \
    .add_handler<type>(                                                 \
        (ve::event_handler) [](const ve::event& ve_impl_event_var) {    \
            const type& event_name = (const type&) ve_impl_event_var;   \
            __VA_ARGS__;                                                \
        }                                                               \
    );


namespace ve {
    class input_manager {
    public:
        // Represents a point in the past from which the mouse state can be obtained.
        enum class recorded_time {
            CURRENT, PREVIOUS,
            MOUSE_LAST_STOPPED, MOUSE_LAST_MOVING,
            WHEEL_LAST_STOPPED, WHEEL_LAST_MOVING
        };
        
        
        [[nodiscard]] static input_manager& instance(void);
        
        [[nodiscard]] bool is_pressed(SDL_KeyCode key) const;
        [[nodiscard]] bool is_pressed(mouse_button button) const;
        
        [[nodiscard]] const key_state& get_state(SDL_Keycode key) const;
        [[nodiscard]] const mouse_state& get_state(recorded_time when = recorded_time::CURRENT) const;
        
        [[nodiscard]] bool is_mouse_moving(void) const;
        [[nodiscard]] vec2i tick_mouse_delta(void) const;       // Mouse motion during the last tick.
        [[nodiscard]] vec2i total_mouse_delta(void) const;      // Mouse motion since the mouse began moving.
    
        [[nodiscard]] bool is_wheel_moving(void) const;
        [[nodiscard]] i32 tick_wheel_delta(void) const;         // Wheel motion during the last tick.
        [[nodiscard]] i32 total_wheel_delta(void) const;        // Wheel motion since the mouse began moving.
        
        VE_GET_MREF(dispatcher);
    private:
        struct mouse_state_history {
            mouse_state last_stopped_state, last_moving_state;
            u64 last_stopped_tick = 0, last_moving_tick = 0;
        };
        
        
        noncancellable_delayed_event_dispatcher dispatcher;
        
        // Mutable since getting the state may require creating it first.
        mutable hash_map<SDL_Keycode, key_state> keyboard_state;
        
        mouse_state mouse_current_state, mouse_last_state;
        mouse_state_history mouse_history;
        mouse_state_history mousewheel_history;
        
        small_flat_map<u32, vec2ui> window_sizes;
        small_flat_map<u32, vec2ui> window_positions;
    
    
        friend class engine;
    
        input_manager(void) = default;
        
        void update(u64 tick);
        
        template <bool is_down> void handle_keyboard_change(SDL_Event& e, u64 tick);
        template <bool is_wheel> void handle_mouse_motion(u64 tick);
        void handle_mouse_buttons(u64 tick);
    };
}