#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <auto Value> using value = std::integral_constant<decltype(Value), Value>;
    template <typename T> using type  = std::type_identity<T>;


    /** Indicates the absence of a type. */
    struct null_type {};
    /** Templated null_type for when multiple distinct null_types are required, e.g. when using conditionally-enabled base classes. */
    template <std::size_t N> struct null_type_for {};


    /** Indicates a type that will be replaced with another type later. */
    struct placeholder_type {};


    /** Converts T to @ref null_type if it is void, to prevent any issues with void's incompleteness. */
    template <typename T> using devoidify = std::conditional_t<
        std::is_void_v<T>,
        null_type,
        T
    >;
}