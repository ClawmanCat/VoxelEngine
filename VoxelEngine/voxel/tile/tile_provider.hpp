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
        [[nodiscard]] const tile_data& operator[](const vec3i& where) const {
            VE_CRTP_CHECK(Derived, operator[]);
            return static_cast<Derived*>(this)->operator[](where);
        }
    
    
        [[nodiscard]] tile_data& operator[](const vec3i& where) {
            return const_cast<tile_data&>(std::as_const(this)->operator[](where));
        }
    
    
        [[nodiscard]] const tile* get_tile(const vec3i& where) const {
            auto& state = (*this)[where];
            return tile_registry::instance().get_tile(state);
        }
        
        
        [[nodiscard]] tile_metadata get_metadata(const vec3i& where) const {
            return (*this)[where].metadata;
        }
        
        
        [[nodiscard]] tile_state get_tile_state(const vec3i& where) const {
            auto& state = (*this)[where];
            
            return tile_state {
                .tile     = tile_registry::instance().get_tile(state),
                .metadata = state.metadata
            };
        }
    };
}