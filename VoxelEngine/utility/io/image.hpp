#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>


namespace ve {
    // TODO: Make it easier to index by index rather than coordinate.
    template <typename Pixel> requires (meta::glm_traits<Pixel>::is_vector || std::is_scalar_v<Pixel>)
    struct image {
        using value_type = Pixel;
        using image_tag  = void;


        std::vector<Pixel> data;
        vec2ui size;


        Pixel& operator[](const vec2ui& where) {
            return data[where.x + where.y * size.x];
        }

        const Pixel& operator[](const vec2ui& where) const {
            return data[where.x + where.y * size.x];
        }


        template <typename Pred> void foreach(Pred pred) {
            std::size_t i = 0;

            for (u32 y = 0; y < size.y; ++y) {
                for (u32 x = 0; x < size.x; ++x) {
                    pred(vec2ui { x, y }, data[i++]);
                }
            }
        }

        template <typename Pred> void foreach(Pred pred) const {
            std::size_t i = 0;

            for (u32 y = 0; y < size.y; ++y) {
                for (u32 x = 0; x < size.x; ++x) {
                    pred(vec2ui { x, y }, data[i++]);
                }
            }
        }


        void clear(Pixel value = Pixel { 0 }) {
            foreach([&] (const auto& pos, auto& pixel) { pixel = value; });
        }
    };


    using RGBA8   = vec4ub;
    using RGBA32F = vec4f;
    using GRAY8   = u8;
    using GRAY32F = f32;

    using image_rgba8   = image<RGBA8>;
    using image_rgba32f = image<RGBA32F>;
    using image_gray8   = image<GRAY8>;
    using image_gray32f = image<GRAY32F>;


    template <typename Pixel = RGBA8>
    inline image<Pixel> filled_image(const vec2ui& size, const Pixel& color) {
        return image<Pixel> {
            .data = std::vector<Pixel>(size.x * size.y, color),
            .size = size
        };
    }
}