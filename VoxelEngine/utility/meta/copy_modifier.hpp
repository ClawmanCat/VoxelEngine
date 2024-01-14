#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T, typename As> using copy_const = std::conditional_t<
        std::is_const_v<As>,
        const T,
        T
    >;


    template <typename T, typename As> using copy_lvalue_reference = std::conditional_t<
        std::is_lvalue_reference_v<As>,
        T&,
        T
    >;


    template <typename T, typename As> using copy_rvalue_reference = std::conditional_t<
        std::is_rvalue_reference_v<As>,
        T&&,
        T
    >;


    template <typename T, typename As> using copy_reference = copy_lvalue_reference<
        copy_rvalue_reference<T, As>,
        As
    >;


    template <typename T, typename As> using copy_const_reference = copy_reference<
        copy_const<T, As>,
        As
    >;


    template <typename T, typename As> constexpr decltype(auto) const_as(T&& value, As& as) {
        if constexpr (std::is_const_v<As>) return fwd(std::as_const(value));
        else return fwd(value);
    }
}