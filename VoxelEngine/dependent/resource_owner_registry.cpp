#include <VoxelEngine/dependent/resource_owner_registry.hpp>


namespace ve {
    resource_owner_registry& resource_owner_registry::instance(void) {
        static resource_owner_registry i { };
        return i;
    }
}