#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    using actor_id = u32;
    
    // An actor id is used by plugins and games using the engine.
    // When a plugin is unloaded, all resources associated to its ID are unloaded too.
    [[nodiscard]] extern actor_id next_actor_id(void);
}