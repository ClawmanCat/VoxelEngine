#pragma once

#include <VoxelEngine/core/core.hpp>

#include <utility>


namespace ve::meta {
    // Allows assignment of different types based on a compile time constant.
    // e.g. auto var = pick<some_condition>("A string!"s, 43)
    // will result in decltype(var) being either std::string or int depending on some_condition.
    template <bool Condition, typename A, typename B>
    constexpr inline decltype(auto) pick(A&& a, B&& b) {
        if constexpr (Condition) return std::forward<A>(a);
        else return std::forward<B>(b);
    }
}