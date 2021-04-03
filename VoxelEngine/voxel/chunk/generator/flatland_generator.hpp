#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>
#include <VoxelEngine/voxel/chunk/generator/generator.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>


namespace ve {
    namespace detail {
        // Allows initialization of terrain layers with either tile_data or tile.
        struct td_wrapper {
            tile_data td;
            
            td_wrapper(tile_data td) : td(td) {}
            td_wrapper(const tile* t) : td(*t) {}
        };
    }
    
    
    using terrain_layer_map = flat_map<i32, tile_data>;
    
    inline terrain_layer_map terrain_layers(std::initializer_list<std::pair<i32, detail::td_wrapper>> args) {
        terrain_layer_map result;
        for (const auto& [k, v] : args) result.insert(std::pair { k, v.td });
        return result;
    }
    
    
    class flatland_generator : public chunk_generator {
    public:
        // Pairs of [height, state]. All tiles above or at the previous height,
        // up to the given height will be set to the provided tile. Tiles above the last height will be set to sky.
        //
        // e.g. given layers = { { 0, Stone }, { 5, Dirt }, { 6 Grass } }, sky = Air
        // the following layers would be produced:
        // [-Infinity, 0] Stone
        // [1, 5] Dirt
        // 6 Grass
        // [7, +Infinity] Air
        flatland_generator(terrain_layer_map&& layers, tile_data sky) : layers(layers), sky(sky) {}
        
        
        [[nodiscard]] unique<chunk> generate(const vec3i& where) override {
            unique<chunk> result = std::make_unique<chunk>();
            
            tile_data layer_data[voxel_settings::chunk_size];
            for (u32 y = 0; y < voxel_settings::chunk_size; ++y) {
                layer_data[y] = get_data_for_height((where.y * (i32) voxel_settings::chunk_size) + (i32) y);
            }
            
            result->foreach([&](const vec3i& pos, u32 index, tile_data& data) {
                data = layer_data[pos.y];
            });
            
            return result;
        }
        
    private:
        terrain_layer_map layers;
        tile_data sky;
        
        
        [[nodiscard]] tile_data get_data_for_height(i32 height) const {
            auto it = layers.lower_bound(height);
            return it == layers.end() ? sky : it->second;
        }
    };
}