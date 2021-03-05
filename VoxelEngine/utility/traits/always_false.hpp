#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T> class always_false {
        struct secret_type {};
    public:
        constexpr static bool value = std::is_same_v<T, secret_type>;
    };
    
    
    template <typename T> constexpr static bool always_false_v = always_false<T>::value;
}