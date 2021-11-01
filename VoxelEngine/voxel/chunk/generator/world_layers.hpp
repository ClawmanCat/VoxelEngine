#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>


namespace ve::voxel {
    // Defines a set of layers that can be used by a generator to create layered worlds.
    class world_layers {
    public:
        using height_t = tilepos::value_type;

        struct layer {
            height_t limit;
            tile_data data;

            ve_field_comparable(layer, limit);
        };


        // Convenience functions to handle tile => data conversion.
        void add_layer(height_t limit, const tile* tile, tile_metadata_t meta = 0) {
            layers.insert(layer { limit, voxel_settings::get_tile_registry().get_state(tile, meta) });
        }

        void set_sky(const tile* tile, tile_metadata_t meta = 0) {
            sky = voxel_settings::get_tile_registry().get_state(tile, meta);
        }


        const tile_data& get_data_for_height(height_t height) {
            layer* layer_for_height = layers.empty() ? nullptr : &*layers.begin();

            for (const auto& [i, layer] : layers | views::enumerate) {
                if (layer.limit > height) break;
                else layer_for_height = &layer;
            }

            return (layer_for_height && layer_for_height->limit >= height)
                ? layer_for_height->data
                : sky;
        }


        VE_GET_CREF(layers);
        VE_GET_CREF(sky);
    private:
        // Each layer runs from the end of the one below it up to limit (inclusive).
        vec_set<layer> layers;
        // This material will be used to fill the world above the topmost layer.
        tile_data sky;
    };
}