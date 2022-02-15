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
}