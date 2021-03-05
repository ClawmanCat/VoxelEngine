#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T> struct this_type {
        using type = std::remove_reference_t<std::remove_pointer_t<T>>;
    };
    
    template <typename T> using this_type_t = typename this_type<T>::type;
}