#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <template <typename...> typename Trait> struct negation {
        template <typename... Ts> struct type {
            constexpr static inline bool value = !Trait<Ts...>::value;
        };
    };


    template <template <typename...> typename... Traits> struct conjunction {
        template <typename... Ts> struct type {
            constexpr static inline bool value = (Traits<Ts...>::value && ...);
        };
    };


    template <template <typename...> typename... Traits> struct disjunction {
        template <typename... Ts> struct type {
            constexpr static inline bool value = (Traits<Ts...>::value || ...);
        };
    };
}