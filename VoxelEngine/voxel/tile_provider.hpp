#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/typedefs.hpp>
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
                return tilepos { max_value<i32>() };
            }
        }


        tile_data& get_data(const tilepos& where) {
            VE_DEBUG_ASSERT(glm::abs(where) < get_extents(), "Attempt to access tile data outside bounds.");

            // Can't use CRTP_CHECK here because of the const overload,
            // if you get an error here, please implement this method in your derived class.
            return Derived::operator[](where);
        }

        const tile_data& get_data(const tilepos& where) const {
            VE_DEBUG_ASSERT(glm::abs(where) < get_extents(), "Attempt to access tile data outside bounds.");

            // Can't use CRTP_CHECK here because of the const overload,
            // if you get an error here, please implement this method in your derived class.
            return Derived::operator[](where);
        }


        tile_state operator[](const tilepos& where) const {
            const auto& state = get_state(where);

            return tile_state {
                voxel_settings::get_tile_registry().get_tile_for_state(state),
                voxel_settings::get_tile_registry().get_effective_metastate(state)
            };
        }
    };
}