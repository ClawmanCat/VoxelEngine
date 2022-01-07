#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve::meta {
    template <typename T> struct function_traits {
        constexpr static bool is_free_function = false;
        constexpr static bool is_mem_function  = false;
        constexpr static bool is_callable      = false;
    };
    
    
    template <typename Ret, typename... Args>
    struct function_traits<fn<Ret, Args...>> {
        constexpr static bool is_free_function = true;
        constexpr static bool is_mem_function  = false;
        constexpr static bool is_callable      = true;
        
        using return_type = Ret;
        using arguments   = pack<Args...>;
        using signature   = Ret(Args...);
    };
    
    
    template <typename Cls, typename Ret, typename... Args>
    struct function_traits<mem_fn<Cls, Ret, Args...>> {
        constexpr static bool is_free_function = false;
        constexpr static bool is_mem_function  = true;
        constexpr static bool is_callable      = true;
        
        using owning_class = Cls;
        using return_type  = Ret;
        using arguments    = pack<Args...>;
        using signature    = Ret(Cls&, Args...);
        
        constexpr static bool is_const = false;
    };
    
    
    template <typename Cls, typename Ret, typename... Args>
    struct function_traits<const_mem_fn<Cls, Ret, Args...>> {
        constexpr static bool is_free_function = false;
        constexpr static bool is_mem_function  = true;
        constexpr static bool is_callable      = true;
        
        using owning_class = Cls;
        using return_type  = Ret;
        using arguments    = pack<Args...>;
        using signature    = Ret(const Cls&, Args...);
        
        constexpr static bool is_const = true;
    };
    
    
    template <typename T> requires requires (T t) { std::function { t }; }
    struct function_traits<T> {
    private:
        template <typename Ret, typename... Args> struct helper {
            explicit helper(std::function<Ret(Args...)>) {}
            
            using return_type = Ret;
            using arguments   = pack<Args...>;
            using signature   = Ret(Args...);
        };
    
        template <typename R, typename... A>
        helper(std::function<R(A...)>) -> helper<R, A...>;
        
    public:
        constexpr static bool is_free_function = false;
        constexpr static bool is_mem_function  = false;
        constexpr static bool is_callable      = true;
        
        using return_type  = typename decltype(helper { std::function { std::declval<T>() } })::return_type;
        using arguments    = typename decltype(helper { std::function { std::declval<T>() } })::arguments;
        using signature    = typename decltype(helper { std::function { std::declval<T>() } })::signature;
    };


    template <typename Fn, std::size_t N>
    using nth_argument = typename function_traits<Fn>::arguments::template get<N>;
}