#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/input_event.hpp>

#include <SDL_keyboard.h>


namespace ve {
    using keymods = std::underlying_type_t<SDL_Keymod>;


    struct key_state {
        SDL_Keycode key;
        keymods mods;

        steady_clock::time_point last_change;
        u64 last_change_tick;

        bool is_down;


        bool has_mod(SDL_Keymod mod) const { return mods & mod; }
    };


    struct ve_input_event(key_down_event) { cref_of<key_state> old_state, new_state; };
    struct ve_input_event(key_up_event)   { cref_of<key_state> old_state, new_state; };
    struct ve_input_event(key_hold_event) { cref_of<key_state> state; };
    // Triggered once on key down, then not at all for some delay, then at a regular interval while the key is down.
    // This mimics the behaviour of holding a key down in a text editor.
    struct ve_input_event(key_type_event) { cref_of<key_state> state; };
}