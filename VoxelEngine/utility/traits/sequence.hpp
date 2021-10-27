#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>


namespace ve::meta {
    template <typename T, typename S1, typename S2>
    constexpr auto sequence_cat(S1, S2) {
        return [] <T... E1> (std::integer_sequence<T, E1...>) {
            return [] <T... E2> (std::integer_sequence<T, E2...>) {
                return std::integer_sequence<T, E1..., E2...>{};
            }(S2{});
        }(S1{});
    }


    // Generator for std::integer_sequence with more flexibility than std::make_index_sequence.
    // Start is inclusive, stop is exclusive.
    template <typename T, T Start, T Stop, T Delta>
    constexpr auto make_sequence(void) {
        using current_t = std::integer_sequence<T, Start>;

        constexpr bool next_would_exceed = (Delta < 0)
            ? (Start + Delta <= Stop)
            : (Start + Delta >= Stop);

        if constexpr (next_would_exceed) return current_t{};
        else return sequence_cat<T>(current_t{}, make_sequence<T, Start + Delta, Stop, Delta>());
    }


    template <typename T, T... Vals>
    constexpr auto reverse_sequence(std::integer_sequence<T, Vals...> seq) {
        if constexpr (sizeof...(Vals) == 0) {
            return std::integer_sequence<T>{};
        } else {
            return [] <T First, T... Rest> (std::integer_sequence<T, First, Rest...>) {
                return sequence_cat<T>(
                    reverse_sequence(std::integer_sequence<T, Rest...>{}),
                    std::integer_sequence<T, First>{}
                );
            }(seq);
        }
    }
}