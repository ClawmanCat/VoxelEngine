#include <VoxelEngine/voxel/tile/tile_registry.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::voxel {
    tile_data tile_registry::register_tile(const tile* tile, bool removable) {
        VE_ASSERT(tile->get_num_states() > 0, "Cannot register tile without any states.");
        VE_ASSERT(tile->id == invalid_tile_id, "Attempt to register the same tile twice.");

        tile->removable = removable;


        auto register_at = [&] (auto& storage, auto id, auto index) {
            storage.resize(std::max((std::size_t) index, storage.size()));

            storage[index] = tile;
            tile->id = id;
            tile->metastate = index;

            return tile_data { tile->id, tile->metastate };
        };


        if (tile->is_stateless()) {
            if (!stateless_tombstones.empty()) {
                auto tombstone = take(stateless_tombstones);
                return register_at(stateless_tiles[tombstone.first], tombstone.first, tombstone.second);
            }

            for (auto [i, stateless_storage] : stateless_tiles | views::enumerate) {
                if (stateless_storage.size() < max_value<tile_metadata_t>) {
                    return register_at(stateless_storage, i, stateless_storage.size());
                }
            }

            VE_ASSERT(false, "Cannot register tile: the registry is full.");
        }

        else {
            if (!stateful_tombstones.empty()) {
                auto tombstone = take(stateful_tombstones);
                return register_at(stateful_tiles, tombstone, tombstone);
            }

            if (stateful_tiles.size() + 1 < invalid_tile_id) {
                return register_at(stateful_tiles, stateful_tiles.size(), stateful_tiles.size());
            }

            VE_ASSERT(false, "Cannot register tile: the registry is full.");
        }


        VE_UNREACHABLE;
    }


    void tile_registry::unregister_tile(const tile* tile) {
        VE_ASSERT(tile->id != invalid_tile_id, "Attempt to unregister tile that was not previously registered.");
        VE_ASSERT(tile->removable, "Cannot remove tile marked as non-removable from the registry.")

        if (tile->is_stateless()) {
            stateless_tiles.at(tile->id).at(tile->metastate) = nullptr;
            stateless_tombstones.emplace_back(tile->id, tile->metastate);
        } else {
            stateful_tiles.at(tile->id) = nullptr;
            stateful_tombstones.push_back(tile->id);
        }

        tile->id = invalid_tile_id;
        tile->metastate = invalid_tile_id;
    }


    void tile_registry::unregister_all(void) {
        tile_id_t last_remaining_stateful = 0;
        for (const tile* t : stateful_tiles) {
            if (t->removable) unregister_tile(t);
            else last_remaining_stateful = std::max(last_remaining_stateful, t->id);
        }

        stateful_tiles.resize(last_remaining_stateful + 1);

        std::erase_if(
            stateful_tombstones,
            [&](auto tombstone) { return tombstone > last_remaining_stateful; }
        );

        stateful_tombstones.shrink_to_fit();


        auto last_remaining_stateless = create_filled_array<voxel_settings::reserved_stateless_tile_ids>(produce((tile_id_t) 0));
        for (auto [i, storage] : stateless_tiles | views::enumerate) {
            for (const tile* t : storage) {
                if (t->removable) unregister_tile(t);
                else last_remaining_stateless[i] = std::max(last_remaining_stateless[i], t->metastate);
            }

            storage.resize(last_remaining_stateless[i] + 1);
        }

        std::erase_if(
            stateless_tombstones,
            [&](const auto& tombstone) { return tombstone.second > last_remaining_stateless[tombstone.first]; }
        );

        stateless_tombstones.shrink_to_fit();
    }


    const tile* tile_registry::get_tile_for_state(const tile_data& td) const {
        bool is_stateless = (td.tile_id < voxel_settings::reserved_stateless_tile_ids);

        if (is_stateless) {
            return stateless_tiles[td.tile_id][td.metadata];
        } else {
            return stateful_tiles[td.tile_id];
        }
    }


    tile_metadata_t tile_registry::get_effective_metastate(const tile_data& td) const {
        return (td.tile_id < voxel_settings ::reserved_stateless_tile_ids) ? td.metadata : 0;
    }


    tile_data tile_registry::get_default_state(const tile* tile) const {
        return tile->is_stateless()
            ? tile_data { tile->id, tile->metastate }
            : tile_data { tile->id, 0 };
    }


    bool tile_registry::is_removable(const tile *tile) const {
        return tile->removable;
    }
}