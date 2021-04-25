#pragma once

#include <type_traits>
#include <span>


namespace ve {
    // Trick for casting lambdas with an auto&&... parameter to a function pointer.
    // Instead of auto&&..., universal auto... can be used.
    template <typename T, typename Stripped = std::remove_cvref_t<T>> concept universal =
        std::is_same_v<T, Stripped> ||
        std::is_same_v<T, Stripped&> ||
        std::is_same_v<T, const Stripped&>;
    
    
    // Allows for spans with both static and dynamic extents.
    template <typename V, typename T, typename VB = std::remove_cvref_t<V>> concept span = std::is_same_v<
        std::span<std::remove_const_t<T>, VB::extent>,
        std::span<std::remove_const_t<typename VB::value_type>, VB::extent>
    > && (std::is_const_v<T> || !std::is_const_v<typename VB::value_type>);
    
    
    // Binds to A and const A.
    template <typename A, typename B> concept maybe_const =
        std::is_same_v<std::add_const_t<A>, std::add_const_t<B>>;
    
    
    // Allows usage of variadic function parameters of a given type without use of a template.
    // e.g. void fn(identity<std::size_t> auto... indices) rather than
    // template <typename... Ts> requires (std::is_same_v<Ts, std::size_t> && ...) void fn(Ts... indices)
    template <typename A, typename B> concept identity = std::is_same_v<A, B>;
    
    
    // Simplified template for std::integral_constant
    template <auto V> using value = std::integral_constant<decltype(V), V>;
    
    
    // Useful for matching universal reference types
    template <typename A, typename B> constexpr static bool is_loosely_same =
        std::is_same_v<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;
    
    
    // Prevents type deduction when it is advisable to specify types explicitly.
    template <typename X, typename Y> concept dont_deduce = std::is_same_v<X, Y>;
}