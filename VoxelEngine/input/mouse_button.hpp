#pragma once

#include <VoxelEngine/core/core.hpp>

#include <SDL_mouse.h>


namespace ve {
    enum class mouse_button : decltype(SDL_BUTTON_LEFT) {
        LEFT   = SDL_BUTTON_LEFT,
        RIGHT  = SDL_BUTTON_RIGHT,
        MIDDLE = SDL_BUTTON_MIDDLE,
        X1     = SDL_BUTTON_X1,
        X2     = SDL_BUTTON_X2
    };
}