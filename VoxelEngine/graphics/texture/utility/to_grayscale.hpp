#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/graphics/texture/utility/luma.hpp>


namespace ve::gfx {
    // Converts an image to grayscale by taking a weighted average of all channels.
    // The output image type has a single channel of the same type as the channel types of the input image.
    // E.g. an image<vec4f> is converted to an image<f32>.
    template <
        typename Pixel,
        typename PixTraits  = meta::glm_traits<Pixel>,
        typename OutPixel   = typename PixTraits::value_type,
        std::size_t Weights = PixTraits::num_rows
    > inline image<OutPixel> to_grayscale(const image<Pixel>& src, const vec<Weights, f32>& weights = LUMA_ITU_BT709_NO_ALPHA) {
        f32 weights_sum = 0;
        for (std::size_t i = 0; i < Weights; ++i) weights_sum += weights[i];

        vec<Weights, f32> weights_normalized = weights / weights_sum;


        auto result = filled_image<OutPixel>(src.size, OutPixel { 0 });

        for (std::size_t i = 0; i < src.data.size(); ++i) {
            f32 pixel_result = 0;

            for (std::size_t w = 0; w < Weights; ++w) {
                pixel_result += weights_normalized[w] * (f32) src.data[i][w];
            }

            result.data[i] = (OutPixel) pixel_result;
        }

        return result;
    }
}