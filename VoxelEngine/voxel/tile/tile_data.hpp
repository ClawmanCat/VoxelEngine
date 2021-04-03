#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>


namespace ve {
    using tile_id       = voxel_settings::tile_id_t;
    using tile_metadata = voxel_settings::tile_metadata_t;
    
    constexpr inline tile_id invalid_tile = 0;
    
    
    struct tile_data {
        tile_id tile;
        tile_metadata metadata;
        
        ve_eq_comparable(tile_data);
        ve_hashable(tile, metadata);
    };
}