#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/chunk/generator/world_layers.hpp>
#include <VoxelEngine/voxel/space/voxel_space.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>


namespace ve::voxel {
    class chunk_generator : protected chunk_access {
    public:
        virtual ~chunk_generator(void) = default;
        virtual unique<chunk> generate(const voxel_space* space, const tilepos& chunkpos) = 0;
    };


    // Simple flatland generator which simply reads from the given world layers to place tiles at each height.
    class flatland_generator : public chunk_generator {
    public:
        explicit flatland_generator(world_layers layers) : chunk_generator(), layers(std::move(layers)) {}


        unique<chunk> generate(const voxel_space* space, const tilepos& chunkpos) override {
            auto result = make_unique<chunk>();
            auto height = [&] (auto y) { return chunkpos.y * tilepos::value_type { voxel_settings::chunk_size } + y; };


            std::vector<tile_data> layer_cache;
            layer_cache.reserve(voxel_settings::chunk_size);

            for (u32 y = 0; y < voxel_settings::chunk_size; ++y) {
                layer_cache.push_back(layers.get_data_for_height(height(y)));
            }


            result->foreach([&] (const auto& pos, tile_data& data) {
                data = layer_cache[pos.y];
            });


            return result;
        }

    private:
        world_layers layers;
    };
}