#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename T> struct member_traits {
        constexpr static bool is_member_pointer = false;
    };


    template <typename Cls, typename T> struct member_traits<mem_var<Cls, T>> {
        constexpr static bool is_member_pointer = true;

        using class_type  = Cls;
        using member_type = T;
    };
}