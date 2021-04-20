#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>


namespace ve {
    struct tile_state {
        const tile* tile;
        tile_metadata metadata;
    };
    
    
    template <typename Derived> struct tile_provider {
        // Deriving classes should only implement these first two methods.
        [[nodiscard]] const tile_data& get(const vec3i& where) const {
            VE_CRTP_CHECK(Derived, get);
            return static_cast<const Derived*>(this)->get(where);
        }
    
        
        void set(const vec3i& where, const tile_data& td) {
            VE_CRTP_CHECK(Derived, set);
            static_cast<Derived*>(this)->set(where, td);
        }
        
        
        // Array like access. space.tiles[{ x, y, z}] = td;
        // TODO: Overload operator[] with wrapping object instead.
        __declspec(property(
            get = get,
            put = set
        )) tile_data tiles[];
        
        
        // Overloads for working with tiles directly rather than tile IDs.
        const tile* get_tile(const vec3i& where) const {
            return tile_registry::instance().get_tile(tiles[where]);
        }
        
        
        tile_state get_state(const vec3i& where) const {
            const auto& td = tiles[where];
            
            auto result = tile_state {
                tile_registry::instance().get_tile(td),
                td.metadata
            };
            
            // If the tile has no states, it could have been optimized to share an ID with other tiles.
            result.metadata *= bool(result.tile->get_num_states());
            
            return result;
        }
        
        
        void set_tile(const vec3i& where, const tile* t, tile_metadata m = 0) {
            auto td = tile_registry::instance().get_default_tile_state(t);
            td.metadata += m; // Either tile is stateless, in which case m = 0, or it has a state, in which case td.metadata = 0.
            
            tiles[where] = td;
        }
        
        
        void set_state(const vec3i& where, const tile_state& state) {
            set_tile(where, state.tile, state.metadata);
        }
    };
}