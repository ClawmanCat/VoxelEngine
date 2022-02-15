#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <std::size_t FirstExtent, std::size_t... Extents>
    constexpr inline void spatial_iterate(auto pred, auto... indices) {
        if constexpr (sizeof...(Extents) == 0) {
            for (std::size_t i = 0; i < FirstExtent; ++i) {
                std::invoke(pred, indices..., i);
            }
        } else {
            for (std::size_t i = 0; i < FirstExtent; ++i) {
                spatial_iterate<Extents...>(pred, indices..., i);
            }
        }
    }
}