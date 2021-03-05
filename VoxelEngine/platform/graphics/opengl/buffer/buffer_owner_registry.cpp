#include <VoxelEngine/platform/graphics/opengl/buffer/buffer_owner_registry.hpp>


namespace ve::graphics {
    buffer_owner_registry& buffer_owner_registry::instance(void) {
        static buffer_owner_registry i { };
        return i;
    }
}