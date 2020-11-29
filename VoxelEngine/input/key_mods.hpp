#pragma once

#include <VoxelEngine/core/core.hpp>

#include <SDL_keyboard.h>

#include <type_traits>


namespace ve {
    using key_mods = std::underlying_type_t<SDL_Keymod>;
}