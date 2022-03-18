#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/color.hpp>


namespace ve::gfx {
    struct combine_image_data {
        const image_rgba8* src;
        vec4b source_channels, dest_channels;
    };


    // Combines several images into one, sampling each destination channel from one or more channels of a source image.
    // Channels without a source will be returned as opaque black.
    // Note that if multiple channels are set as source, those channels will be converted to grayscale first,
    // before being written to the destination channel(s).
    // If the source and destination images have different sizes, nearest-neighbour sampling will be performed.
    inline image_rgba8 combine_images(const std::vector<combine_image_data>& images, const vec2ui& size) {
        for (u8 channel = 0; channel < 4; ++channel) {
            VE_ASSERT(
                ranges::accumulate(images | views::transform(ve_get_field(dest_channels[channel])), (std::size_t) 0) <= 1,
                "At most one source image can be used to populate a channel in the destination image."
            );
        }


        image_rgba8 result { .data = std::vector<RGBA8>(size.x * size.y, RGBA8 { 0 }), .size = size };

        auto to_grayscale = [](const RGBA8& pixel, const vec4b& mask) {
            const u8 mask_sum = mask.r + mask.g + mask.b + mask.a;

            return u8((
                (u16(pixel.r) * mask.r) +
                (u16(pixel.g) * mask.g) +
                (u16(pixel.b) * mask.b) +
                (u16(pixel.a) * mask.a)
            ) / mask_sum);
        };


        for (const auto& [src, src_channels, dest_channels] : images) {
            if (src->size == vec2ui { 1 }) {
                // Handle the semi-common case of a single-pixel image separately, since it can be handled more cheaply.
                for (u8 channel = 0; channel < 4; ++channel) {
                    if (!dest_channels[channel]) continue;

                    u8 value = to_grayscale(src->data[0], src_channels);
                    for (std::size_t i = 0; i < src->data.size(); ++i) result.data[i][channel] = value;
                }
            }

            else if (src->size == size) {
                // Source and destination have the same size, no scaling required.
                for (u8 channel = 0; channel < 4; ++channel) {
                    if (!dest_channels[channel]) continue;

                    for (std::size_t i = 0; i < src->data.size(); ++i) {
                        result.data[i][channel] = to_grayscale(src->data[i], src_channels);
                    }
                }
            }

            else {
                // Sizes differ, use a sampler to read from the image.

                // Cannot use structured bindings inside lambda.
                const auto& ref_src_img = *src;
                const auto& ref_src_channels = src_channels;

                auto sampler = [&, factor = vec2f { src->size } / vec2f { size }] (const vec2ui& where) {
                    return ref_src_img[vec2ui { vec2f { where } * factor }];
                };

                for (u8 channel = 0; channel < 4; ++channel) {
                    if (!dest_channels[channel]) continue;
                    result.foreach([&] (const auto& where, auto& pixel) { pixel[channel] = to_grayscale(sampler(where), ref_src_channels); });
                }
            }
        }

        return result;
    }


    inline image_rgba8 make_material_texture(
        const image_rgba8& roughness,
        const image_rgba8& metalness,
        const image_rgba8& ambient_occlusion,
        const image_rgba8& emissivity,
        vec2ui size = vec2ui { 0 }
    ) {
        // If no size was provided, use the largest component texture size.
        if (size == vec2ui { 0 }) {
            size.x = std::max({ roughness.size.x, metalness.size.x, ambient_occlusion.size.x, emissivity.size.x });
            size.y = std::max({ roughness.size.y, metalness.size.y, ambient_occlusion.size.y, emissivity.size.y });
        }


        return combine_images({
            combine_image_data { .src = &roughness,         .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 1, 0, 0, 0 } },
            combine_image_data { .src = &metalness,         .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 0, 1, 0, 0 } },
            combine_image_data { .src = &ambient_occlusion, .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 0, 0, 1, 0 } },
            combine_image_data { .src = &emissivity,        .source_channels = { 1, 1, 1, 0 }, .dest_channels = { 0, 0, 0, 1 } }
        }, size);
    }
}