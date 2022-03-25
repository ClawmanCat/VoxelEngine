#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/graphics/texture/utility/luma.hpp>
#include <VoxelEngine/graphics/texture/utility/resize.hpp>
#include <VoxelEngine/graphics/texture/utility/to_grayscale.hpp>


namespace ve::gfx {
    template <typename Pixel> struct combine_image_data {
        const image<Pixel>* src;
        vec4b source_channels, dest_channels;
    };


    // Combines several images into one, sampling each destination channel from one or more channels of a source image.
    // Channels without a source will be set to 0.
    // Note that if multiple channels are set as source, those channels will be converted to grayscale first before being written to the destination channel(s).
    // If the source and destination images have different sizes, resampling will be performed using the provided sampler. Sampler pixel type should be Pixel::value_type.
    // TODO: This method should allow for differing pixel types per image, e.g. combining 4 grayscale images into one RGBA one.
    template <typename Pixel, typename Sampler = image_samplers::nearest_neighbour<typename meta::glm_traits<Pixel>::value_type>>
    inline image<Pixel> combine_images(
        const std::vector<combine_image_data<Pixel>>& images,
        const vec2ui& size,
        Sampler sampler = Sampler { }
    ) {
        using pix_traits = meta::glm_traits<Pixel>;
        using weight_vec = vec<pix_traits::num_rows, f32>;


        for (u8 channel = 0; channel < 4; ++channel) {
            VE_ASSERT(
                ranges::accumulate(images | views::transform(ve_get_field(dest_channels[channel])), (std::size_t) 0) <= 1,
                "At most one source image can be used to populate a channel in the destination image."
            );

            VE_ASSERT(
                ranges::all_of(images, [&] (const auto& i) { return channel < pix_traits::num_rows || i.source_channels[channel] == 0; }),
                "Cannot sample from channel ", channel, " in image with pixel type ", ctti::nameof<Pixel>(), " which has only ", pix_traits::num_rows, " channels."
            );

            VE_ASSERT(
                ranges::all_of(images, [&] (const auto& i) { return channel < pix_traits::num_rows || i.dest_channels[channel] == 0; }),
                "Cannot write to channel ", channel, " in image with pixel type ", ctti::nameof<Pixel>(), " which has only ", pix_traits::num_rows, " channels."
            );
        }


        auto result = filled_image(size, Pixel { 0 });

        for (const auto& [src, src_channels, dest_channels] : images) {
            auto gray_value = to_grayscale(*src, weight_vec { src_channels });


            // Handle the semi-common case of a single-pixel image separately, since it can be handled more cheaply.
            if (src->data.size() == 1) {
                auto fill_pixel = Pixel { gray_value.data[0] } * Pixel { dest_channels };
                result.foreach([&] (const auto& pos, auto& pixel) {
                    pixel += fill_pixel;
                });
            }

            // Source and destination have the same size, no scaling required.
            else if (src->size == size) {
                for (std::size_t i = 0; i < src->data.size(); ++i) {
                    result.data[i] += gray_value.data[i] * Pixel { dest_channels };
                }
            }

            // Sizes differ, use sampler to read from the image.
            else {
                const vec2f scale = vec2f { src->size } / vec2f { result.size };
                std::size_t i = 0;

                for (std::size_t y = 0; y < result.size.y; ++y) {
                    for (std::size_t x = 0; x < result.size.x; ++x) {
                        result.data[i++] += sampler(gray_value, vec2ui { x, y }, scale, result.size) * Pixel { dest_channels };
                    }
                }
            }
        }

        return result;
    }
}