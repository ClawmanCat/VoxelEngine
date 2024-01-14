#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <template <typename...> typename Tmpl, typename T> struct is_template {
    private:
        template <typename... Xs> constexpr static inline std::true_type  test(const Tmpl<Xs...>&);
        template <typename... Xs> constexpr static inline std::false_type test(...);
    public:
        constexpr static inline bool value = decltype(test(std::declval<T>()))::value;
    };


    template <template <typename...> typename Tmpl> struct is_template<Tmpl, void> {
        constexpr static inline bool value = false;
    };


    template <template <typename...> typename Tmpl, typename T>
    constexpr inline bool is_template_v = is_template<Tmpl, T>::value;


    template <typename T, template <typename...> typename Tmpl>
    concept template_instantiation_of = is_template_v<Tmpl, T>;
}