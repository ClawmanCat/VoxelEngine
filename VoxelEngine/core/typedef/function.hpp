#pragma once

#include <type_traits>


namespace ve {
    namespace defs {
        template <typename Ret, typename... Args>
        using fn = Ret(*)(Args...);
    
        template <typename Cls, typename Ret, typename... Args>
        using mem_fn = Ret(Cls::*)(Args...);
    
        template <typename Cls, typename Ret, typename... Args>
        using const_mem_fn = Ret(Cls::*)(Args...) const;
    
        template <typename Cls, typename T>
        using mem_var = T(Cls::*);
        
        template <typename T>
        using ref = std::reference_wrapper<T>;


        template <typename Cls, typename Ret, typename... Args>
        using maybe_const_mem_fn = std::conditional_t<
            std::is_const_v<Cls>,
            const_mem_fn<Cls, Ret, Args...>,
            mem_fn<Cls, Ret, Args...>
        >;
    }
    
    using namespace defs;
}