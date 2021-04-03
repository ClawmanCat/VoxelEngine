#include <VoxelEngine/voxel/tile/simple_tile_storage.hpp>


namespace ve::detail {
    simple_tile_storage& ve_tile_storage(void) {
        static simple_tile_storage instance;
        return instance;
    }
}