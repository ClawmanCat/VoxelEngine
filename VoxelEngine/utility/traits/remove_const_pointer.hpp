#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    // Equivalent to std::remove_cvref_t, but for pointers instead of references.
    template <typename T> using remove_const_pointer = std::remove_const_t<std::remove_pointer_t<T>>;
}