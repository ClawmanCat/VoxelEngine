#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>


namespace ve::meta {
    template <typename T, T... S1, T... S2>
    constexpr auto sequence_cat(std::integer_sequence<T, S1...>, std::integer_sequence<T, S2...>) {
        return std::integer_sequence<T, S1..., S2...>{};
    }
    
    
    // Generator for std::integer_sequence with more flexibility than std::make_index_sequence.
    template <typename T, T Start, T Stop, T Delta>
    constexpr auto make_sequence(void) {
        constexpr auto op = pick<(Delta > 0)>(std::greater_equal<T>{}, std::less<T>{});
        
        if constexpr (op(Start + Delta, Stop)) {
            return std::integer_sequence<T>{};
        } else {
            return sequence_cat(
                std::integer_sequence<T, Start>{},
                make_sequence<T, Start + Delta, Stop, Delta>()
            );
        }
    }
}