#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>


namespace ve::meta {
    template <typename Fn> struct member_function_traits {
        constexpr static bool is_member_function = false;
    };
    
    
    template <typename Cls, typename Ret, typename... Args> struct member_function_traits<MemFn<Cls, Ret, Args...>> {
        constexpr static bool is_member_function = true;
        constexpr static bool is_const = false;
        
        using containing_class_type = Cls;
        using return_type           = Ret;
        using argument_types        = pack<Args...>;
        using freed_pointer_type    = Fn<Ret, void*, Args...>;
    };
    
    
    template <typename Cls, typename Ret, typename... Args> struct member_function_traits<ConstMemFn<Cls, Ret, Args...>> {
        constexpr static bool is_member_function = true;
        constexpr static bool is_const = true;
        
        using containing_class_type = Cls;
        using return_type           = Ret;
        using argument_types        = pack<Args...>;
        using freed_pointer_type    = Fn<Ret, const void*, Args...>;
    };
    
    
    template <typename T> using mft = member_function_traits<T>;
}