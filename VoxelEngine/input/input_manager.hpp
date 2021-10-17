#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/input/keyboard.hpp>
#include <VoxelEngine/input/mouse.hpp>
#include <VoxelEngine/input/window.hpp>

#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>


namespace ve {
    // TODO: Support joystick and controller input events.
    // TODO: Handle double clicking.
    class input_manager : public subscribe_only_view<simple_event_dispatcher<false>> {
    public:
        using custom_event_handler = std::function<void(const SDL_Event&)>;

        struct custom_handler_handle {
            SDL_EventType type;
            typename std::list<custom_event_handler>::const_iterator it;
        };


        static input_manager& instance(void);


        void update(u64 tick);


        // Provide some way of adding external handlers, since the input manager will clear the event queue,
        // even for unhandled events.
        custom_handler_handle add_custom_handler(SDL_EventType type, custom_event_handler&& handler);
        void remove_custom_handler(custom_handler_handle handle);


        // Provide some way to create fake events. This is required e.g. for handling window_create events,
        // as SDL provides no event for this.
        template <typename Event> void trigger_event(const Event& event) {
            dispatch_event(event);
        }


        const key_state& get_key_state(SDL_Keycode key) const;
        bool is_key_pressed(SDL_Keycode key) const;

        const mouse_button_state& get_mouse_button_state(mouse_button button) const;
        bool is_mouse_button_pressed(mouse_button button) const;

        keymods get_current_keymods(void) const;

        void set_mouse_capture(bool enabled);
        bool has_mouse_capture(void) const;

        VE_GET_CREF(current_mouse_move);
        VE_GET_CREF(current_mouse_drag);
        VE_GET_CREF(current_mouse_wheel_move);
        VE_GET_CREF(current_mouse_state);
        VE_GET_CREF(prev_mouse_state);
    private:
        // Mutable since get_key_state should work, even on keys that haven't had any events yet.
        mutable hash_map<SDL_Keycode, key_state> keyboard_state;

        struct ongoing_mouse_event { mouse_state begin_state; };
        std::optional<ongoing_mouse_event> current_mouse_move;
        std::optional<ongoing_mouse_event> current_mouse_wheel_move;
        std::array<std::optional<ongoing_mouse_event>, magic_enum::enum_count<mouse_button>()> current_mouse_drag;

        mouse_state current_mouse_state, prev_mouse_state;

        struct per_window_data { vec2ui size; gfx::window::window_location location; };
        vec_map<SDL_Window*, per_window_data> window_data;


        hash_map<std::underlying_type_t<SDL_EventType>, std::list<custom_event_handler>> custom_handlers;


        key_state& get_mutable_key_state(SDL_Keycode key) const;
        mouse_button_state& get_mutable_mouse_button_state(mouse_button button);

        void handle_key_event         (SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now);
        void handle_mouse_button_event(SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now);
        void handle_mouse_move_event  (SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now);
        void handle_mouse_wheel_event (SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now);
        void handle_window_event      (SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now);

    };
}