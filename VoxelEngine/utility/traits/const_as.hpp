#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename As, typename T> using const_as = std::conditional_t<
        std::is_const_v<std::remove_reference_t<As>>,
        T,
        const T
    >;
}