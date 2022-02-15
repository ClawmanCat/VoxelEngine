#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>

#include <FastNoise/FastNoise.h>
#include <FastSIMD/FastSIMD.h>


namespace ve::noise {
    using noise_source = FastNoise::SmartNode<>;

    // TODO: Make this a setting.
    constexpr inline auto SIMD_instruction_set = FastSIMD::Level_AVX2;


    // Loads noise from a file. Use the FastNoise2 NoiseTool to generate noise files.
    inline auto from_file(const fs::path& path) {
        auto encoded_noise = cat_range_with(io::read_text(path), "\n");
        return FastNoise::NewFromEncodedNodeTree(encoded_noise.c_str(), SIMD_instruction_set);
    }


    template <typename NoiseFn> inline auto create(void) {
        return FastNoise::New<NoiseFn>(SIMD_instruction_set);
    }
}