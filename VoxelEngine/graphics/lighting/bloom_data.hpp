#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx {
    struct bloom_data {
        constexpr static inline vec3f LUMA_ITU_BT709 = vec3f { 0.2126f, 0.7152f, 0.0722f };
        constexpr static inline vec3f LUMA_ITU_BT601 = vec3f { 0.2990f, 0.5870f, 0.1440f };

        vec3f luma_conversion_weights = LUMA_ITU_BT709;
        float bloom_intensity = 1.0f;
        float bloom_threshold = 1.0f;
    };
}