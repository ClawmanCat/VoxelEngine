#include <VoxelEngine/dependent/actor.hpp>


namespace ve {
    actor_id next_actor_id(void) {
        static actor_id next_id = 0;
        return next_id++;
    }
}