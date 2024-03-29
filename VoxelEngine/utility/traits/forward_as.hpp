#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    namespace detail {
        template <typename As, typename T>
        using forward_as_t = std::conditional_t<
            std::is_lvalue_reference_v<As>,
            std::remove_reference_t<T>&,
            std::remove_reference_t<T>&&
        >;
    }


    template <typename As, typename T>
    constexpr detail::forward_as_t<As, T> forward_as(T&& value) {
        return std::forward<detail::forward_as_t<As, T>>(std::forward<T>(value));
    }
}