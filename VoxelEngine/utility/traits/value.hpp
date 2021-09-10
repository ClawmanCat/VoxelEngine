#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    // Wrapper around std::integral_constant which automatically deduces the contained type.
    template <auto V> using value = std::integral_constant<decltype(V), V>;
    
    // Can be passed as an argument to templated lambdas, so they may be called as l(type_wrapper<C>{}),
    // rather than l.template operator()<C>().
    template <typename T> struct type_wrapper { using type = T; };
}