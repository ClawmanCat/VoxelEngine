#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>
#include <VoxelEngine/utility/logger.hpp>



namespace ve {
    class tile_registry {
    public:
        static tile_registry& instance(void);
        
        
        tile_data register_tile(const tile* tile) {
            VE_ASSERT(tile->get_num_states() > 0, "Cannot register tile without any states.");
            VE_ASSERT(tiles.size() < std::numeric_limits<u16>::max(), "The tile registry is full.");
            
            
            tile_data result;
            
            if (
                tile->get_num_states() == 1 &&
                !(stateless_tiles.size() == stateless_alloc && current_stateless_meta_full())
            ) {
                if (current_stateless_meta_full()) stateless_tiles.emplace_back();
                stateless_tiles.back().emplace_back(tile);
                
                result = tile_data {
                    .tile = u16(stateless_tiles.size() - 1),
                    .metadata = u16(stateless_tiles.back().size() - 1)
                };
            } else {
                tiles.emplace_back(tile);
                result = tile_data { .tile = u16(tiles.size() - 1), .metadata = 0 };
            }
            
            
            tile->id = result.tile;
            tile->default_state = result.metadata;
            
            return result;
        }
        
        
        void unregister_tile(const tile* tile) {
            VE_ASSERT(false, "Not yet implemented");
        }
        
        
        [[nodiscard]] const tile* get_tile(const tile_data& state) const {
            if (state.tile < stateless_alloc) {
                return stateless_tiles[state.tile][state.metadata];
            } else {
                return tiles[state.tile];
            }
        }
        
        
        [[nodiscard]] tile_data get_default_tile_state(const tile* tile) const {
            return tile_data {
                .tile     = tile->id,
                .metadata = tile->default_state
            };
        }
    private:
        tile_registry(void) {
            stateless_tiles.resize(1);
        }
        
        
        constexpr static inline std::size_t stateless_alloc = voxel_settings::tile_registry_stateless_alloc;
        
        std::vector<const tile*> tiles;
        std::vector<std::vector<const tile*>> stateless_tiles;
        
        [[nodiscard]] bool current_stateless_meta_full(void) const {
            return stateless_tiles.back().size() >= std::numeric_limits<tile_metadata>::max();
        }
    };
}