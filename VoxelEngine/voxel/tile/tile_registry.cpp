#include <VoxelEngine/voxel/tile/tile_registry.hpp>


namespace ve {
    tile_registry& tile_registry::instance(void) {
        static tile_registry i { };
        return i;
    }
}