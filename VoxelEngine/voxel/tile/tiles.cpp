#include <VoxelEngine/voxel/tile/tiles.hpp>
#include <VoxelEngine/voxel/tile/simple_tile_storage.hpp>


namespace ve::tiles {
    const tile* TILE_VOID = detail::ve_tile_storage().emplace<tile>(tile_parameters {
        .name       = "void",
        .colliding  = false,
        .rendered   = false,
        .num_states = 1,
    });
    
    
    const tile* TILE_UNKNOWN = detail::ve_tile_storage().emplace<tile>(tile_parameters {
        .name       = "unknown",
        .colliding  = false,
        .rendered   = false,
        .num_states = 1,
    });
}