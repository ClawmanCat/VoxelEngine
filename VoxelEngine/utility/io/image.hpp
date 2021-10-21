#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    using RGBA8 = vec4ub;
    static_assert(sizeof(RGBA8) == 4 * sizeof(u8));


    struct image_rgba8 {
        std::vector<RGBA8> data;
        vec2ui size;


        RGBA8& operator[](const vec2ui& where) {
            return data[where.x + where.y * size.x];
        }

        const RGBA8& operator[](const vec2ui& where) const {
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
    };


    inline image_rgba8 filled_image(const vec2ui& size, const RGBA8& color) {
        return image_rgba8 {
            .data = std::vector<RGBA8>(size.x * size.y, color),
            .size = size
        };
    }
}