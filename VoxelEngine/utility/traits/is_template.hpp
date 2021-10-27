#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    // Checks if T is a specialization of template Tmpl.
    template <template <typename...> typename Tmpl, typename T>
    class is_template {
    private:
        template <typename... Xs> constexpr static auto test(const Tmpl<Xs...>&) -> std::true_type;
        constexpr static auto test(...) -> std::false_type;
    public:
        constexpr static bool value = decltype(test(std::declval<T>()))::value;
    };
    
    
    template <template <typename...> typename Tmpl, typename T>
    constexpr inline bool is_template_v = is_template<Tmpl, T>::value;
}