#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/input_event.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <SDL_mouse.h>
#include <SDL_keyboard.h>
#include <magic_enum.hpp>


namespace ve {
    using keymods = std::underlying_type_t<SDL_Keymod>;


    enum class mouse_button : u32 { LEFT, RIGHT, MIDDLE, X1, X2 };
    
    
    constexpr inline mouse_button button_from_sdl(u32 button) {
        switch (button) {
            case SDL_BUTTON_LEFT:   return mouse_button::LEFT;  
            case SDL_BUTTON_RIGHT:  return mouse_button::RIGHT;
            case SDL_BUTTON_MIDDLE: return mouse_button::MIDDLE;
            case SDL_BUTTON_X1:     return mouse_button::X1;
            case SDL_BUTTON_X2:     return mouse_button::X2;
        }
        
        VE_UNREACHABLE;
    }


    constexpr inline u32 button_to_sdl(mouse_button button) {
        switch (button) {
            case mouse_button::LEFT:   return SDL_BUTTON_LEFT;
            case mouse_button::RIGHT:  return SDL_BUTTON_RIGHT;
            case mouse_button::MIDDLE: return SDL_BUTTON_MIDDLE;
            case mouse_button::X1:     return SDL_BUTTON_X1;
            case mouse_button::X2:     return SDL_BUTTON_X2;
        }

        VE_UNREACHABLE;
    }


    struct mouse_button_state {
        mouse_button button;
        keymods mods;

        steady_clock::time_point last_change;
        u64 last_change_tick;

        bool is_down;


        bool has_mod(SDL_Keymod mod) const { return mods & mod; }
    };


    struct mouse_state {
        std::array<mouse_button_state, magic_enum::enum_count<mouse_button>()> buttons;

        vec2i   position;
        i32     wheel_position;
        keymods mods;
    };


    struct ve_input_event(button_press_event)   { cref_of<mouse_button_state> old_state, new_state; };
    struct ve_input_event(button_release_event) { cref_of<mouse_button_state> old_state, new_state; };
    struct ve_input_event(button_hold_event)    { cref_of<mouse_button_state> state; };

    // Triggered every tick while the mouse is moving.
    // begin_state is the state when the mouse first started moving, old_state was the state last tick, new_state / end_state is the current state.
    struct ve_input_event(mouse_moved_event)      { cref_of<mouse_state> begin_state, old_state, new_state; };
    // Triggers when the mouse first starts moving and when it stops moving respectively.
    struct ve_input_event(mouse_move_start_event) { cref_of<mouse_state> begin_state; };
    struct ve_input_event(mouse_move_end_event)   { cref_of<mouse_state> begin_state, end_state; };

    // Equivalent to above, but for the mouse wheel.
    struct ve_input_event(mouse_wheel_moved_event)      { cref_of<mouse_state> begin_state, old_state, new_state; };
    struct ve_input_event(mouse_wheel_move_start_event) { cref_of<mouse_state> begin_state; };
    struct ve_input_event(mouse_wheel_move_end_event)   { cref_of<mouse_state> begin_state, end_state; };

    struct ve_input_event(mouse_drag_event)       { mouse_button button; cref_of<mouse_state> begin_state, old_state, new_state; };
    struct ve_input_event(mouse_drag_start_event) { mouse_button button; cref_of<mouse_state> begin_state; };
    struct ve_input_event(mouse_drag_end_event)   { mouse_button button; cref_of<mouse_state> begin_state, end_state; };


    // TODO: Consider redoing event hierarchy so we don't need this.
    template <typename Event> constexpr inline auto& get_most_recent_state(Event& e) {
        if      constexpr (requires { e.end_state;   }) return e.end_state;
        else if constexpr (requires { e.new_state;   }) return e.new_state;
        else if constexpr (requires { e.state;       }) return e.state;
        else if constexpr (requires { e.begin_state; }) return e.begin_state;
        else static_assert(meta::always_false_v<Event>, "Event does not have a state field.");
    }

    template <typename Event> constexpr inline auto& get_previous_state(Event& e) {
        if      constexpr (requires { e.old_state;   }) return e.old_state;
        else if constexpr (requires { e.begin_state; }) return e.begin_state;
        else if constexpr (requires { e.state;       }) return e.state;
        else static_assert(meta::always_false_v<Event>, "Event does not have a state field.");
    }

    template <typename Event> constexpr inline auto& get_begin_state(Event& e) {
        if      constexpr (requires { e.begin_state; }) return e.begin_state;
        else if constexpr (requires { e.old_state;   }) return e.old_state;
        else if constexpr (requires { e.state;       }) return e.state;
        else static_assert(meta::always_false_v<Event>, "Event does not have a state field.");
    }
}