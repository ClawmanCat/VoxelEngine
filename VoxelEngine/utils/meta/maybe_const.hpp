#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T, bool Const> using maybe_const_t = std::conditional_t<Const, const T, T>;
    
    template <typename A, typename B> concept maybe_const = std::is_same_v<const A, const B>;
}