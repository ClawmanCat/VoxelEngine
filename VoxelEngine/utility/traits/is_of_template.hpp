#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <template <typename...> typename Tmpl, typename T>
    class is_of_template {
        template <typename... Args> constexpr static auto test(const Tmpl<Args...>*) -> std::true_type;
        template <typename...> constexpr static auto test(...) -> std::false_type;
    
    public:
        constexpr static bool value = decltype(test(std::declval<T*>()))::value;
    };
    
    
    template <template <typename...> typename Tmpl, typename T>
    constexpr static bool is_of_template_v = is_of_template<Tmpl, T>::value;
}