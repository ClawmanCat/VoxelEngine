#include <VoxelEngine/ecs/component/partially_synchronizable.hpp>


namespace ve::detail {
    ps_message_registry& ps_message_registry::instance(void) {
        static ps_message_registry i { };
        return i;
    }
}