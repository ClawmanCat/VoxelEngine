#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename T, typename As>
    struct reference_as {
        using type = std::remove_reference_t<T>;
    };

    template <typename T, typename As>
    struct reference_as<T, As&> {
        using type = std::add_lvalue_reference_t<std::remove_reference_t<T>>;
    };

    template <typename T, typename As>
    struct reference_as<T, As&&> {
        using type = std::add_rvalue_reference_t<std::remove_reference_t<T>>;
    };


    template <typename T, typename As>
    using reference_as_t = typename reference_as<T, As>::type;
}