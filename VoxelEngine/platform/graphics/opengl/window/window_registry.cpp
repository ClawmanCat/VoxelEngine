#include <VoxelEngine/platform/graphics/opengl/window/window_registry.hpp>


namespace ve::graphics {
    window_registry& window_registry::instance(void) {
        static window_registry i { };
        return i;
    }
}