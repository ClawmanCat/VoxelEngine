#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>


namespace ve::tiles {
    // A tile used to indicate the absence of a real tile.
    const extern tile* TILE_VOID;
    // A tile used to indicate that the tile is not loaded.
    const extern tile* TILE_UNKNOWN;
}