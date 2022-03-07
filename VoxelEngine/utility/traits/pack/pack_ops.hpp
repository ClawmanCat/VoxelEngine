#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve::meta::pack_ops {
    // Predicates for mapping a pack of packs to each nth type.
    template <std::size_t N> struct nth {
        template <typename... Packs> using type = pack<typename Packs::template get<N>...>;
    };

    template <typename... Packs> using keys   = typename nth<0>::template type<Packs...>;
    template <typename... Packs> using values = typename nth<1>::template type<Packs...>;


    // Zips two or more packs into a pack of packs.
    // E.g. given two packs A = pack<T1, T2, T3>, B = pack<T4, T5, T6> returns pack<pack<T1, T4>, pack<T2, T5>, pack<T3, T6>>.
    template <typename First, typename... Packs> requires ((Packs::size == First::size) && ...)
    struct zip {
        template <std::size_t N> using all_nths = pack<typename First::template get<N>, typename Packs::template get<N>...>;

        constexpr static auto combine_types(void) {
            return [] <std::size_t... Is> (std::index_sequence<Is...>) {
                return pack<all_nths<Is>...>{};
            }(std::make_index_sequence<First::size>());
        }

        using type = decltype(combine_types());
    };


    template <typename First, typename... Packs> struct intersection_impl {
        template <typename T> struct filter {
            constexpr static inline bool value = (Packs::template contains<T> && ...);
        };

        using type = typename First::template filter_trait<filter>;
    };

    // Returns a new pack containing only the types that are present in every provided pack.
    template <typename... Ts> using intersection = typename intersection_impl<Ts...>::type;


    template <typename T, typename... Ts> struct merge_all_impl {
        template <typename X> using as_pack = std::conditional_t<
            requires { typename X::pack_tag; }, X, pack<X>
        >;

        constexpr static auto impl(void) {
            if constexpr (sizeof...(Ts) == 0) {
                return as_pack<T>{};
            } else {
                using tail_t = decltype(merge_all_impl<Ts...>::impl());
                return typename as_pack<T>::template append_pack<tail_t>{};
            }
        }

        using type = decltype(impl());
    };

    // Creates a pack that is the result of merging all packs and non-pack types in Ts.
    // E.g. returns pack<A, B, C, D, E> if Ts = [pack<A, B>, C, pack<D, E>]
    template <typename... Ts> using merge_all = typename merge_all_impl<Ts...>::type;


    template <typename First, typename... Packs> struct pack_permutation_impl {
        template <typename Pack> constexpr static bool checker =
            First::size == Pack::size &&
            First::all([] <typename T> { return Pack::template contains<T>; });

        constexpr static bool value = (checker<Packs> && ...);
    };

    // Returns true if all given packs are permutations of each other, that is, they all contain the same types,
    // but in a possibly differing order.
    template <typename... Packs> constexpr static bool is_permutation = pack_permutation_impl<Packs...>::value;
}