#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>


namespace ve::gfx {
    constexpr inline u32 GAUSSIAN_HORIZONTAL = 0;
    constexpr inline u32 GAUSSIAN_VERTICAL   = 1;


    template <std::size_t WeightLimit = 16> struct gaussian_blur_data {
        constexpr static inline std::size_t weight_count_limit = WeightLimit;

        std::array<float, WeightLimit> weights;
        u32 populated_weights = 0;
        float range = 1.0f;
    };


    inline std::vector<float> make_gaussian_weights(std::size_t count, float stddev = 2.0f) {
        auto gaussian = [] (float x, float sigma) {
            float a = 1.0f / std::sqrt(2.0f * constants::f32_pi * sigma * sigma);
            float b = std::exp(-((x * x) / (2.0f * sigma * sigma)));
            return a * b;
        };

        std::vector<float> result;
        result.reserve(count);

        for (std::size_t i = 0; i < count; ++i) result.push_back(gaussian(float(i), stddev));

        return result;
    }
}