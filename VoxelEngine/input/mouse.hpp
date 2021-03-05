#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/tick_timepoint.hpp>
#include <VoxelEngine/input/keyboard.hpp>

#include <magic_enum.hpp>
#include <SDL_mouse.h>

#include <array>


namespace ve {
    enum class mouse_button {
        LEFT,
        RIGHT,
        MIDDLE,
        X1,
        X2
    };
    
    constexpr inline mouse_button button_from_sdl(auto button) {
        if (button == SDL_BUTTON_LEFT)   return mouse_button::LEFT;
        if (button == SDL_BUTTON_RIGHT)  return mouse_button::RIGHT;
        if (button == SDL_BUTTON_MIDDLE) return mouse_button::MIDDLE;
        if (button == SDL_BUTTON_X1)     return mouse_button::X1;
        if (button == SDL_BUTTON_X2)     return mouse_button::X2;
        
        VE_UNREACHABLE;
    }
    
    
    
    struct mouse_button_state {
        mouse_button button;
        
        bool down = false;
        key_mods mods = SDL_Keymod::KMOD_NONE;
        tick_timepoint last_down = {}, last_up = {};
    };
    
    
    struct mouse_state {
        vec2i position;
        i32   wheel_position;
        
        std::array<
            mouse_button_state,
            magic_enum::enum_count<mouse_button>()
        > buttons;
    };
}