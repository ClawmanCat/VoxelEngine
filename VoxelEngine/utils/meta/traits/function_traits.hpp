#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/pack.hpp>


namespace ve::meta {
    template <typename> struct function_traits {};
    
    
    template <typename Ret, typename... Args> struct function_traits<Fn<Ret, Args...>> {
        using return_type    = Ret;
        using argument_types = pack<Args...>;
    };
    
    
    template <typename Cls, typename Ret, typename... Args> struct function_traits<MemFn<Cls, Ret, Args...>> {
        using return_type    = Ret;
        using argument_types = pack<Args...>;
        using class_type     = Cls;
        
        constexpr static bool is_const = false;
    };
    
    
    template <typename Cls, typename Ret, typename... Args> struct function_traits<ConstMemFn<Cls, Ret, Args...>> {
        using return_type    = Ret;
        using argument_types = pack<Args...>;
        using class_type     = Cls;
        
        constexpr static bool is_const = true;
    };
}