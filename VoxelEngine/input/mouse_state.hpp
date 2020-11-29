#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/key_mods.hpp>

#include <SDL_keyboard.h>
#include <glm/glm.hpp>

#include <chrono>


namespace ve {
    struct mouse_state {
        vec2i position;
        i32   wheel_position;
    };
}