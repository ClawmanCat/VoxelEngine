#include <VoxelEngine/voxel/tile/tiles.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>
#include <VoxelEngine/voxel/settings.hpp>


namespace ve::voxel::tiles {
    const tile* TILE_AIR = detail::create(tile::arguments {
        .name     = "air",
        .rendered = false
    });

    const tile* TILE_UNKNOWN = detail::create(tile::arguments {
        .name     = "unknown tile",
        .rendered = false
    });


    namespace detail {
        const tile* create(const tile::arguments& args) {
            auto& ptr = tile_storage.emplace_back(make_unique<tile>(args));

            // Mark this tile as non-removable since it is used by the engine itself.
            voxel_settings::get_tile_registry().register_tile(ptr.get(), false);

            return ptr.get();
        }
    }
}