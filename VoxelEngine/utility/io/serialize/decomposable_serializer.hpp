#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/decompose.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/traits/bind.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/traits/evaluate_if_valid.hpp>

#include <boost/pfr.hpp>


namespace ve::serialize {
    template <typename T> void to_bytes(const T&, std::vector<u8>&);
    template <typename T> T from_bytes(std::span<const u8>&);


    namespace detail {
        // Can T be directly initialized with parameters matching each of its member variables?
        // Note: this can cause false positives when T has a constructor that accepts the required parameters,
        // but uses them for something else than direct initialization.
        // For this reason, the member assignment method is preferred, even if it avoids direct initialization.
        template <typename T, typename Pack = meta::create_pack::from_decomposable<T>>
        constexpr inline bool supports_direct_initialization_v =
            Pack::template expand_inside<
                meta::bind_types<std::is_constructible, T>::template front
            >::value &&
            Pack::all([] <typename E> { return std::is_move_constructible_v<T>; });


        // Can T be default constructed and then have all its members assigned to afterwards?
        template <typename T, typename Pack = meta::create_pack::from_decomposable<T>>
        constexpr inline bool supports_member_assignment_v =
            std::is_default_constructible_v<T> &&
            Pack::all([] <typename E> { return std::is_move_assignable_v<T>; });
    }


    template <typename T> requires is_decomposable_v<T>
    inline void decomposable_to_bytes(const T& value, std::vector<u8>& dest) {
        // We want to pop in the reverse order we pushed, since the last element will be at the end of the array.
        // Since its easier to push in reverse than to pop in reverse, just flip the element order here.
        using member_types = typename meta::create_pack::from_decomposable<T>::reverse;


        member_types::foreach_indexed([&] <typename E, std::size_t I> {
            constexpr std::size_t EI = member_types::size - I - 1; // We're iterating in reverse!

            const E& member = decomposer_for<T>::template get<EI>(value);
            to_bytes<E>(member, dest);
        });
    }


    template <typename T> requires is_decomposable_v<T>
    inline T decomposable_from_bytes(std::span<const u8>& src) {
        using member_types = typename meta::create_pack::from_decomposable<T>
            ::template expand_outside<std::remove_const_t>;


        if constexpr (detail::supports_member_assignment_v<T>) {
            T result { };

            member_types::foreach_indexed([&] <typename E, std::size_t I> {
                E& member = decomposer_for<T>::template get<I>(result);
                member = from_bytes<std::remove_cvref_t<E>>(src);
            });

            return result;
        } else {
            // Note: guaranteed to be sequenced correctly: https://eel.is/c++draft/dcl.init.list#4
            return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                return T { from_bytes<typename member_types::template get<Is>>(src)... };
            } (std::make_index_sequence<member_types::size>());
        }
    }
}