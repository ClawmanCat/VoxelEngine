#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    // Helper for creating variables of different type depending on some constexpr condition.
    // E.g.: auto x = pick<Cond>(a, b); where a and b are of different types.
    // Note that unlike with a constexpr-if statement, both conditions are still evaluated.
    template <bool Cond, typename A, typename B>
    constexpr decltype(auto) pick(A&& a, B&& b) {
        if constexpr (Cond) return fwd(a);
        else return fwd(b);
    }


    // Equivalent to above, but the false condition is not evaluated.
    #define ve_unevaluated_pick(cond, A, B)         \
    [&] () -> decltype(auto) {                      \
        if constexpr (cond) return A;               \
        else return B;                              \
    }()
}