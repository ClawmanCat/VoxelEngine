#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>


namespace ve::voxel {
    constexpr inline tile_id_t invalid_tile_id = max_value<tile_id_t>;


    struct tile_data {
        tile_id_t tile_id;
        // It is valid to use an empty type as the metadata type if metadata support is not needed.
        [[no_unique_address]] tile_metadata_t metadata;


        ve_eq_comparable(tile_data);
        ve_hashable();
    };
}