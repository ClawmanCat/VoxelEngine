#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename T> class is_function_ptr {
        template <typename TT = T, typename Ret, typename... Args> constexpr static auto test(int)
            -> decltype((Fn<Ret, Args...>) std::declval<TT>(), std::true_type());
        
        template <typename...> constexpr static auto test(...) -> std::false_type;
    public:
        constexpr static bool value = decltype(test(0))::value;
    };
    
    
    template <typename T> constexpr static bool is_function_ptr_v = is_function_ptr<T>::value;
}