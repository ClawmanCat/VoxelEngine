#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T> struct reference_as {
        template <typename X> using type = std::remove_reference_t<X>;
    };
    
    template <typename T> struct reference_as<T&> {
        template <typename X> using type = std::add_lvalue_reference_t<std::remove_reference_t<X>>;
    };
    
    template <typename T> struct reference_as<T&&> {
        template <typename X> using type = std::add_rvalue_reference_t<std::remove_reference_t<X>>;
    };
    
    
    template <typename MaybeRef, typename T>
    using reference_as_t = typename reference_as<MaybeRef>::template type<T>;
}