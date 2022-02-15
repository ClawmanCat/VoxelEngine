#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename T> constexpr static bool is_immovable_v =
        !std::is_copy_constructible_v<T> &&
        !std::is_copy_assignable_v<T>    &&
        !std::is_move_constructible_v<T> &&
        !std::is_move_assignable_v<T>;
}