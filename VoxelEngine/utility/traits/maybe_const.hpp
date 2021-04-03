#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/reference_as.hpp>

#include <type_traits>


namespace ve::meta {
    template <bool as_const, typename T> using maybe_const = std::conditional_t<
        as_const,
        // std::add_const does not work for references.
        reference_as_t<
            T,
            std::add_const_t<std::remove_reference_t<T>>
        >,
        T
    >;
}