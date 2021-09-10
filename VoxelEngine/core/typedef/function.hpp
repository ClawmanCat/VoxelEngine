#pragma once

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
    }
    
    using namespace defs;
}