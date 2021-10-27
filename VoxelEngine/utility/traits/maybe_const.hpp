#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename X, typename Y> concept maybe_const = std::is_same_v<
        std::add_const_t<X>,
        std::add_const_t<Y>
    >;
}