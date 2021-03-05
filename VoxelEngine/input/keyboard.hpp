#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/tick_timepoint.hpp>

#include <SDL_keyboard.h>

#include <type_traits>


namespace ve {
    using key_mods = std::underlying_type_t<SDL_Keymod>;
    
    
    struct key_state {
        SDL_Keycode key;
        
        bool down = false;
        key_mods mods = SDL_Keymod::KMOD_NONE;
        tick_timepoint last_down = {}, last_up = {};
    };
}