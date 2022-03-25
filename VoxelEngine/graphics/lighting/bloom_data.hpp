#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/utility/utility.hpp>


namespace ve::gfx {
    struct bloom_data {
        vec3f luma_conversion_weights = LUMA_ITU_BT709;
        float bloom_intensity = 1.0f;
        float bloom_threshold = 1.0f;
    };
}