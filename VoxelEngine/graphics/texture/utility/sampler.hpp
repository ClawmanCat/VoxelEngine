#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>


namespace ve::gfx::image_samplers {
    template <typename Pixel> struct nearest_neighbour {
        constexpr static inline float epsilon = 1e-6f;


        Pixel operator()(const image<Pixel>& src, const vec2ui& dest, const vec2f& factor, const vec2ui& dest_size) const {
            return src[glm::clamp(
                vec2ui { glm::round(vec2f { dest } * factor - epsilon) },
                vec2ui { 0 },
                src.size - vec2ui { 1 }
            )];
        }
    };


    template <typename Pixel> struct bilinear {
        Pixel operator()(const image<Pixel>& src, const vec2ui& dest, const vec2f& factor, const vec2ui& dest_size) const {
            using pix_traits = meta::glm_traits<Pixel>;
            using float_vec  = vec<
                pix_traits::num_rows,
                std::conditional_t<std::is_floating_point_v<typename pix_traits::value_type>, typename pix_traits::value_type, f32>
            >;


            auto lerp = [] (auto a, auto b, auto t) { return a + (b - a) * t; };

            auto bi_lerp = [&] (auto p00, auto p10, auto p01, auto p11, auto tx, auto ty) {
                return lerp(lerp(p00, p10, tx), lerp(p01, p11, tx), ty);
            };


            u32 fx = std::clamp((u32) std::floor((f32) dest.x * factor.x), (u32) 0, src.size.x);
            u32 cx = std::clamp((u32) std::ceil ((f32) dest.x * factor.x), (u32) 0, src.size.x);
            u32 fy = std::clamp((u32) std::floor((f32) dest.y * factor.y), (u32) 0, src.size.y);
            u32 cy = std::clamp((u32) std::ceil ((f32) dest.y * factor.y), (u32) 0, src.size.y);

            return (Pixel) bi_lerp(
                (float_vec) src[{ fx, fy }], (float_vec) src[{ cx, fy }], (float_vec) src[{ fx, cy }], (float_vec) src[{ cx, cy }],
                ((f32) dest.x * factor.x) - (f32) fx,
                ((f32) dest.y * factor.y) - (f32) fy
            );
        }
    };


    // Simply resizes without keeping the original image data. Useful if the image is resized then immediately overwritten.
    template <typename Pixel> struct discard_data {
        Pixel operator()(const image<Pixel>& src, const vec2ui& dest, const vec2f& factor, const vec2ui& dest_size) const {
            return Pixel { 0 };
        }
    };
}