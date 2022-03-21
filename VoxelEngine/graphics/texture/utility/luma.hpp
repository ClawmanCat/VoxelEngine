#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/then.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>


namespace ve::gfx {
    // Weighted averages for converting pixels to brightness values (https://en.wikipedia.org/wiki/Luma_(video)).
    constexpr static inline vec3f LUMA_ITU_BT709   = vec3f { 0.2126f, 0.7152f, 0.0722f };
    constexpr static inline vec3f LUMA_ITU_BT601   = vec3f { 0.2990f, 0.5870f, 0.1440f };
    constexpr static inline vec3f LUMA_RGB_AVERAGE = vec3f { 0.3333f, 0.3333f, 0.3333f };


    // Equivalent to above, but for images with alpha channels (alpha is ignored).
    constexpr static inline vec4f LUMA_ITU_BT709_NO_ALPHA   = vec4f { 0.2126f, 0.7152f, 0.0722f, 0.0000f };
    constexpr static inline vec4f LUMA_ITU_BT601_NO_ALPHA   = vec4f { 0.2990f, 0.5870f, 0.1440f, 0.0000f };
    constexpr static inline vec4f LUMA_RGB_AVERAGE_NO_ALPHA = vec4f { 0.3333f, 0.3333f, 0.3333f, 0.0000f };


    // Equivalent to LUMA_RGB_AVERAGE, but with a number of channels equal to that of the provided pixel type.
    template <typename Pixel> constexpr static inline auto LUMA_PIXEL_AVERAGE =
        vec<meta::glm_traits<Pixel>::num_rows, f32> { 1.0f / (f32) meta::glm_traits<Pixel>::num_rows };

    template <typename Pixel> constexpr static inline auto LUMA_PIXEL_AVERAGE_NO_ALPHA =
        vec<meta::glm_traits<Pixel>::num_rows, f32> { 1.0f / (f32) (meta::glm_traits<Pixel>::num_rows - 1) }
        | then([] (auto& weights) { weights[meta::glm_traits<Pixel>::num_rows - 1] = 0.0f; });
}