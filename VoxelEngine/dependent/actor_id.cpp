#include <VoxelEngine/dependent/actor_id.hpp>
#include <VoxelEngine/threading/threadsafe.hpp>


namespace ve {
    [[nodiscard]] actor_id next_actor_id(void) {
        ve_threadsafe_function;
        
        static actor_id next_id = 0;
        return next_id++;
    }
}