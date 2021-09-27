#include <VoxelEngine/graphics/shader/cache.hpp>


namespace ve::gfx {
    shader_cache& shader_cache::instance(void) {
        static shader_cache i { };
        return i;
    }
}