#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/entity/entity.hpp>
#include <VoxelEngine/entity/component/component_fn.hpp>
#include <VoxelEngine/utils/container_utils.hpp>


namespace ve {
    class world : public entity {
        static inline entt::registry reg { };
        
    public:
        world(void) : entity(&reg) {}
        
        int x, y;
        std::string z;
        
        VE_DYNAMIC_COMPONENT_FN(world, update, void, (float, dt)) {
            VE_LOG_DEBUG("Default update");
        }
        
        
        void do_thing_a(int x, int y) {}
        int get_thing_b(void) { return 3; }
        
        
        VE_DATA_COMPONENT_LIST(entity, x, y, z);
        VE_FN_COMPONENT_LIST(entity, do_thing_a, get_thing_b);
    };
}