#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>


namespace ve::voxel::tiles {
    // Indicates the tile is empty.
    extern const tile* TILE_AIR;
    // Indicates the tile data cannot be retrieved, e.g. when attempting to fetch an unloaded tile.
    extern const tile* TILE_UNKNOWN;


    namespace detail {
        inline std::vector<unique<tile>> tile_storage = { };
        extern const tile* create(const tile::arguments& args);
    }
}