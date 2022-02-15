#pragma once

#include <VoxelEngine/core/core.hpp>

#include <ratio>


namespace ve::meta {
    namespace detail {
        template <typename T> struct is_ratio {
            constexpr static bool value = false;
        };

        template <std::intmax_t N, std::intmax_t D> struct is_ratio<std::ratio<N, D>> {
            constexpr static bool value = true;
        };
    }


    template <typename T> concept ratio = detail::is_ratio<T>::value;
}