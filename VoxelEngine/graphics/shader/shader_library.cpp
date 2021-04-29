#include <VoxelEngine/graphics/shader/shader_library.hpp>


namespace ve::graphics {
    shader_library& shader_library::instance(void) {
        static shader_library i;
        return i;
    }
}