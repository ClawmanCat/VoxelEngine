#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Wraps T such that it is move-only and moving the object swaps the value with the moved object's value.
    template <typename T> struct swap_movable {
        T value;

        explicit swap_movable(T value = T { }) : value(std::move(value)) {}
        ve_swap_move_only(swap_movable, value);
        ve_dereference_as(value);
    };
}