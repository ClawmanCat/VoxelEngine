#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>


namespace ve::voxel {
    class tile_registry {
    public:
        tile_data register_tile(const tile* tile, bool removable = true);
        void unregister_tile(const tile* tile);
        void unregister_all(void);

        const tile* get_tile_for_state(const tile_data& td) const;
        tile_metadata_t get_effective_metastate(const tile_data& td) const;
        tile_data get_default_state(const tile* tile) const;
        tile_data get_state(const tile* tile, tile_metadata_t meta) const;
        bool is_removable(const tile* tile) const;
    private:
        // TODO: Use entt::basic_storage here?
        std::vector<const tile*> stateful_tiles;
        std::vector<tile_id_t> stateful_tombstones;

        std::array<std::vector<const tile*>, voxel_settings::reserved_stateless_tile_ids> stateless_tiles;
        std::vector<std::pair<tile_id_t, tile_id_t>> stateless_tombstones;
    };
}