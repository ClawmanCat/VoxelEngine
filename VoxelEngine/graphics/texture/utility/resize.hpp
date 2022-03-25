#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/graphics/texture/utility/sampler.hpp>


namespace ve::gfx {
    // Resizes the provided image to the requested size using the provided sampler.
    template <typename Pixel> inline image<Pixel> resize_image(const image<Pixel>& source, const vec2ui& resize_to, auto sampler = image_samplers::nearest_neighbour<Pixel>{}) {
        auto result = filled_image<Pixel>(resize_to, Pixel { 0 });
        const vec2f factor = vec2f { source.size } / vec2f { result.size };

        // Skip unnecessary work, in case the optimizer doesn't catch this.
        if constexpr (std::is_same_v<decltype(sampler), image_samplers::discard_data<Pixel>>) return result;

        u32 i = 0;
        for (u32 y = 0; y < resize_to.y; ++y) {
            for (u32 x = 0; x < resize_to.x; ++x) {
                result.data[i++] = std::invoke(sampler, source, vec2ui { x, y }, factor, resize_to);
            }
        }

        return result;
    }
}