#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/pack.hpp>

#include <type_traits>
#include <utility>


namespace ve::meta {
    template <auto find, auto... values> struct sequence_contains {
        constexpr static bool value = false;
    };
    
    template <auto find, auto first, auto... rest> struct sequence_contains<find, first, rest...> {
        constexpr static bool value = (find == first) || sequence_contains<find, rest...>::value;
    };
    
    
    template <auto find, auto... values, typename T = typename pack<decltype(values)...>::head>
    constexpr inline bool sequence_contains_v(const std::integer_sequence<T, values...> seq) noexcept {
        return sequence_contains<find, values...>::value;
    }
}