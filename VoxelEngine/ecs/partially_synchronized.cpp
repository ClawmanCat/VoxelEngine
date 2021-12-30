#include <VoxelEngine/ecs/partially_synchronized.hpp>


namespace ve::detail {
    partially_synchronized_type_registry& partially_synchronized_type_registry::instance(void) {
        static partially_synchronized_type_registry i { };
        return i;
    }
}