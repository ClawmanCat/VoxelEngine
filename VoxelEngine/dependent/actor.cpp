#include <VoxelEngine/dependent/actor.hpp>

#include <cmath>


namespace ve {
    [[nodiscard]] actor_id next_actor_id(void) {
        static actor_id next_id = std::max(engine_actor_id, game_actor_id) + 1;
        return next_id++;
    }
}