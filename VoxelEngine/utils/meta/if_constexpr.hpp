#pragma once

#include <VoxelEngine/core/core.hpp>

#include <utility>


namespace ve::meta {
    template <bool Condition, typename A, typename B>
    constexpr inline decltype(auto) return_if(A&& a, B&& b) {
        if constexpr (Condition) return std::forward<A>(a);
        else return std::forward<B>(b);
    }
}