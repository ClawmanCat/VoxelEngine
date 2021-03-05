#pragma once

#include <VoxelEngine/core/core.hpp>

#include <limits>


namespace ve {
    using actor_id = u32;
    
    
    constexpr inline actor_id engine_actor_id = 0;
    constexpr inline actor_id game_actor_id   = 1;
    constexpr inline actor_id no_actor_id     = std::numeric_limits<actor_id>::max();
    
    
    [[nodiscard]] extern actor_id next_actor_id(void);
}