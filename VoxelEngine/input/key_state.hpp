#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/key_mods.hpp>

#include <SDL_keyboard.h>

#include <chrono>


namespace ve {
    struct key_state {
        SDL_Keycode keycode;
        bool is_down = false;
        
        // The keymods active last time this key was pressed down.
        key_mods mods = SDL_Keymod::KMOD_NONE;
        
        // Time for last key down / key up in ticks and as a timestamp.
        u64 last_down_tick = 0, last_up_tick = 0;
        steady_clock::time_point last_down_time = steady_epoch, last_up_time = steady_epoch;
    };
}