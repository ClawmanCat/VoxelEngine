#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T> struct is_const_member_function_pointer : public std::false_type {};
    
    template <typename C, typename R, typename... A>
    struct is_const_member_function_pointer<ConstMemFn<C, R, A...>> : public std::true_type {};
    
    
    template <typename T>
    constexpr static bool is_const_member_function_pointer_v = is_const_member_function_pointer<T>::value;
}