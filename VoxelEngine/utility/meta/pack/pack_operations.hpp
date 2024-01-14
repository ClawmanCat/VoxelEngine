#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/meta/pack/create_pack.hpp>


namespace ve::meta {
    namespace detail {
        template <type_pack Current, type_pack P0, type_pack... Rest> constexpr inline void cartesian_product_impl(auto&& fn) {
            if constexpr (sizeof...(Rest) == 0) {
                P0::foreach([&] <typename T> {
                    Current::apply([&] <typename... Ts> {
                        fn.template operator()<Ts..., T>();
                    });
                });
            } else {
                P0::foreach([&] <typename T> {
                    cartesian_product_impl<
                        typename Current::template append<T>,
                        Rest...
                    >(fwd(fn));
                });
            }
        }


        template <type_pack... Packs>
        consteval inline auto intersection_impl(void) { return meta::pack<>{}; }

        template <type_pack P0, type_pack... Packs>
        consteval inline auto intersection_impl(void) {
            constexpr auto filter = [] <typename P> { return (Packs::template contains<P> && ...); };
            return typename P0::template filter_value<filter>{};
        }


        template <type_pack P0, type_pack P1> consteval inline auto difference_impl(void) {
            constexpr auto filter = [] <typename T> { return !P1::template contains<T>; };
            return typename P0::template filter_value<filter>{};
        }
    }


    /**
     * Invokes fn<Ts...>() for the cartesian product of the given packs, i.e. every possible combination of one type from each pack.
     * @tparam Packs A set of type packs.
     * @param fn A function invocable as fn<Ts...>().
     */
    template <type_pack... Packs> constexpr inline void pack_cartesian_product(auto&& fn) {
        if constexpr (sizeof...(Packs) > 0) detail::cartesian_product_impl<pack<>, Packs...>(fwd(fn));
    }


    /** Returns the intersection of the given packs. That is, all types which are in all of the given packs. */
    template <type_pack... Packs> using pack_intersection = decltype(detail::intersection_impl<Packs...>());
    /** Returns the union of the given packs. That is, all types which are in at least one of the given packs. */
    template <type_pack... Packs> using pack_union = typename create_pack::from_many<Packs...>::template unique<>;
    /** Returns the difference of the given packs. That is, all types which are in pack A, but not in pack B. */
    template <type_pack A, type_pack B> using pack_difference = decltype(detail::difference_impl<A, B>());
}