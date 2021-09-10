#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename X, typename Y> concept is_type = std::is_same_v<X, Y>;
}