#include <VoxelEngine/ecs/component_registry.hpp>


namespace ve {
    component_registry& component_registry::instance(void) {
        static component_registry i { };
        return i;
    }
}