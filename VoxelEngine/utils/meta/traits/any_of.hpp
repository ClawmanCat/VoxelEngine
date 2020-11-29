#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T, typename... Ts> struct is_any_of {
        constexpr static bool value = (std::is_same_v<T, Ts> || ...);
    };
    
    template <typename T, typename... Ts> constexpr static bool is_any_of_v = is_any_of<T, Ts...>::value;
    template <typename T, typename... Ts> concept is_any_of_c = is_any_of_v<T, Ts...>;
    
    
    // Equivalent to is_any_of but returns true if T is convertible to a type in Ts.
    template <typename T, typename... Ts> struct is_loosely_any_of {
        constexpr static bool value = (std::is_convertible_v<Ts, T> || ...);
    };
    
    template <typename T, typename... Ts> constexpr static bool is_loosely_any_of_v = is_loosely_any_of<T, Ts...>::value;
    template <typename T, typename... Ts> concept is_loosely_any_of_c = is_loosely_any_of_v<T, Ts...>;
}