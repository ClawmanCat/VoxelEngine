#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/chunk/generator/generator.hpp>
#include <VoxelEngine/voxel/chunk/generator/world_layers.hpp>
#include <VoxelEngine/utility/noise.hpp>


namespace ve::voxel {
    // Generates chunks using a noise function as height map.
    // World layers are sampled such that a height of 0 is at the surface.
    class simple_noise_generator : public chunk_generator {
    public:
        struct arguments {
            noise::noise_source heightmap;
            world_layers layers;

            i32 seed = 0;
            // Noise scale in the XZ direction.
            float frequency = 0.005f;
            // Noise scale in the Y direction.
            float height_scale = 100.0f;
        };


        simple_noise_generator(arguments args) :
            heightmap(std::move(args.heightmap)),
            layers(std::move(args.layers)),
            seed(args.seed),
            frequency(args.frequency),
            height_scale(args.height_scale)
        {}


        unique<chunk> generate(const voxel_space* space, const tilepos& chunkpos) override {
            auto result      = make_unique<chunk>();
            auto chunk_start = chunkpos * tilepos::value_type { voxel_settings::chunk_size };


            std::vector<float> chunk_heightmap(square(voxel_settings::chunk_size), 0.0f);

            heightmap->GenUniformGrid2D(
                chunk_heightmap.data(),
                chunk_start.x,
                chunk_start.z,
                voxel_settings::chunk_size,
                voxel_settings::chunk_size,
                frequency,
                seed
            );


            result->foreach([&] (const auto& pos, tile_data& data) {
                // Position at which to sample the heightmap. Flattened XZ in chunk-local coordinates.
                auto sample_height = pos.z * tilepos::value_type { voxel_settings::chunk_size } + pos.x;
                // Height coordinate of the current voxel in global space.
                auto height_base = chunkpos.y * tilepos::value_type { voxel_settings::chunk_size } + pos.y;

                data = layers.get_data_for_height(height_base - ((world_layers::height_t) height_scale * chunk_heightmap[sample_height]));
            });


            return result;
        }
    private:
        noise::noise_source heightmap;
        world_layers layers;
        i32 seed;
        float frequency;
        float height_scale;
    };
}