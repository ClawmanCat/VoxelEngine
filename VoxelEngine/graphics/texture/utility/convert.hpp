#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>


namespace ve::gfx {
    namespace detail {
        // Integral types: assume the entire range is used.
        // Floating point types: assume values are normalized.
        template <typename VT> constexpr inline auto image_value_range = std::is_integral_v<VT>
            ? vec<2, VT> { min_value<VT>, max_value<VT> }
            : vec<2, VT> { VT { 0 }, VT { 1 } };
    }


    template <
        typename From,
        typename To,
        typename FVT = typename meta::glm_traits<From>::value_type,
        typename TVT = typename meta::glm_traits<To>::value_type
    > inline image<To> convert_image(const image<From>& src, vec<2, FVT> src_range = detail::image_value_range<FVT>, vec<2, TVT> dst_range = detail::image_value_range<TVT>) {
        auto result = filled_image<To>(src.size, To { 0 });

        for (std::size_t i = 0; i < src.data.size(); ++i) {
            result[i] = TVT { scale<f32>(src[i], vec2f { src_range }, vec2f { dst_range }) };
        }

        return result;
    }
}