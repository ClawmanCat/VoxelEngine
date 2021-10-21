#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/space/voxel_space.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>


namespace ve::voxel {
    class chunk_generator : protected chunk_access {
    public:
        virtual ~chunk_generator(void) = default;
        virtual unique<chunk> generate(const voxel_space* space, const tilepos& chunkpos) = 0;
    };


    class flatland_generator : public chunk_generator {
    public:
        struct world_layers {
            using height_t = tilepos::value_type;

            // Each layer runs from the end of the one below it up to limit (inclusive).
            struct layer { height_t limit; tile_data data; };
            std::vector<layer> layers;

            // This material will be used to fill the world above the topmost layer.
            tile_data sky;


            // Convenience functions to handle tile => data conversion.
            void add_layer(height_t limit, const tile* tile, tile_metadata_t meta = 0) {
                layers.push_back(layer { limit, voxel_settings::get_tile_registry().get_state(tile, meta) });
            }

            void set_sky(const tile* tile, tile_metadata_t meta = 0) {
                sky = voxel_settings::get_tile_registry().get_state(tile, meta);
            }
        };


        explicit flatland_generator(world_layers layers) : chunk_generator(), layers(std::move(layers)) {
            ranges::sort(layers.layers, ranges::less { }, ve_get_field(limit));
        }


        unique<chunk> generate(const voxel_space* space, const tilepos& chunkpos) override {
            auto result = make_unique<chunk>();


            std::vector<tile_data*> layer_cache;
            layer_cache.resize(voxel_settings::chunk_size, nullptr);

            result->foreach([&] (const auto& pos, tile_data& data) {
                auto height = chunkpos.y * voxel_settings::chunk_size + pos.y;

                if (layer_cache[pos.y]) {
                    data = *layer_cache[pos.y];
                } else {
                    auto it = ranges::lower_bound(
                        layers.layers,
                        height,
                        std::less { },
                        ve_get_field(limit)
                    );

                    layer_cache[pos.y] = (it == layers.layers.end()) ? &layers.sky : &it->data;
                    data = *layer_cache[pos.y];
                }
            });


            return result;
        }

    private:
        world_layers layers;
    };
}