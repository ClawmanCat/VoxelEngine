#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/value.hpp>


namespace ve::meta::detail {
    // Convert between T and T*. Useful when a pack method attempts to return an incomplete pack type.
    template <typename T> constexpr inline T* empointer = std::add_pointer_t<T> { nullptr };
    template <typename T> using depointer = std::remove_pointer_t<T>;


    template <template <typename...> typename P, typename... Ts>
    struct pack_splitter {
        using head = null_type;
        using tail = P<>;
    };

    template <template <typename...> typename P, typename T0, typename... Ts>
    struct pack_splitter<P, T0, Ts...> {
        using head = T0;
        using tail = P<Ts...>;
    };




    template <template <typename...> typename P, typename T0, typename Pack> struct pack_append_type { };

    template <template <typename...> typename P, typename T0, typename... Ts> struct pack_append_type<P, T0, P<Ts...>> {
        using type = P<Ts..., T0>;
    };

    template <template <typename...> typename P, typename... Ts> struct pack_reverser {
        using type = P<>;
    };

    template <template <typename...> typename P, typename T0, typename... Ts> struct pack_reverser<P, T0, Ts...> {
        using type = typename pack_append_type<
            P,
            T0,
            typename pack_reverser<P, Ts...>::type
        >::type;
    };




    template <auto F> struct predicate_wrapper {
        template <typename T> constexpr decltype(auto) operator()(void) const {
            return F.template operator()<T>();
        }
    };

    template <template <typename...> typename Trait> struct trait_wrapper {
        template <typename T> constexpr decltype(auto) operator()(void) const {
            return Trait<T>::value;
        }
    };
}