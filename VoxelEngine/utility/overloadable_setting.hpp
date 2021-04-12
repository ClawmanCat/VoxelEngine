#pragma once

#include <VoxelEngine/core/core.hpp>


// Allows the definition of setting classes within the engine that can be overloaded from within the game.
// The overloaded class should only define those settings that it changes from the defaults,
// as the default will automatically be used if no overload is provided.
#define VE_OVERLOAD_SETTINGS(Cls) \
template <> struct Cls<ve::overloaded_settings_tag> : public Cls<void>


namespace ve {
    struct overloaded_settings_tag {};
}