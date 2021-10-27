#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <std::size_t N, typename Pred>
    constexpr inline void repeat(Pred pred) {
        [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            (pred.template operator()<Is>(), ...);
        }(std::make_index_sequence<N>());
    }
}