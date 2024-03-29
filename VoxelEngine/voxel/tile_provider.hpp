#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>


namespace ve::voxel {
    struct tile_state {
        const tile* tile;
        tile_metadata_t meta;
    };


    template <typename Derived> struct tile_provider {
        constexpr static bool is_bounded(void) {
            if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED(Derived, is_bounded)) {
                return Derived::is_bounded();
            } else {
                return false;
            }
        }


        constexpr static tilepos get_extents(void) {
            if constexpr (is_bounded()) {
                VE_STATIC_CRTP_CHECK(Derived, get_extents);
                return Derived::get_extents();
            } else {
                return tilepos { max_value<tilepos::value_type> };
            }
        }


        const tile_data& get_data(const tilepos& where) const {
            VE_DEBUG_ASSERT(glm::all(glm::abs(where) < get_extents()), "Attempt to access tile data outside bounds.");

            VE_CRTP_CHECK(Derived, get_data);
            return static_cast<const Derived*>(this)->get_data(where);
        }


        // Returns the old data of the voxel.
        tile_data set_data(const tilepos& where, const tile_data& td) {
            VE_CRTP_CHECK(Derived, set_data);
            return static_cast<Derived*>(this)->set_data(where, td);
        }


        tile_state get_state(const tilepos& where) const {
            const auto& data = get_data(where);

            return tile_state {
                voxel_settings::get_tile_registry().get_tile_for_state(data),
                voxel_settings::get_tile_registry().get_effective_metastate(data)
            };
        }


        // Returns the old state of the voxel.
        tile_state set_state(const tilepos& where, const tile_state& state) {
            auto prev_data = set_data(
                where,
                voxel_settings::get_tile_registry().get_state(state.tile, state.meta)
            );

            return tile_state {
                .tile = voxel_settings::get_tile_registry().get_tile_for_state(prev_data),
                .meta = voxel_settings::get_tile_registry().get_effective_metastate(prev_data)
            };
        }
    };
}