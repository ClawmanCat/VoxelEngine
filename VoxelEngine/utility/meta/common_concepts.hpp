#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    // Expressions of the form { T::member } -> std::same_as<MemberT> do not work since decltype(T::member) will be MemberT&.
    // These concepts can be used instead.
    template <typename L, typename R> concept member_type       = std::same_as<L, R&>;
    template <typename L, typename R> concept const_member_type = std::same_as<L, const R&>;


    template <typename T> concept reference         = std::is_lvalue_reference_v<T> || std::is_rvalue_reference_v<T>;
    template <typename T> concept mutable_reference = reference<T> && !std::is_const_v<std::remove_reference_t<T>>;
    template <typename T> concept const_reference   = reference<T> &&  std::is_const_v<std::remove_reference_t<T>>;


    template <typename...> struct all_same_t { constexpr static inline bool value = true; };
    template <typename T, typename... Ts> struct all_same_t<T, Ts...> { constexpr static inline bool value = (std::is_same_v<T, Ts> && ...); };

    template <typename...> struct all_different_t { constexpr static inline bool value = true; };
    template <typename T, typename... Ts> struct all_different_t<T, Ts...> { constexpr static inline bool value = ((!std::is_same_v<T, Ts>) && ...); };


    template <typename T, typename... Ts> constexpr inline bool one_of        = (std::is_same_v<T, Ts> || ...);
    template <typename... Ts>             constexpr inline bool all_same      = all_same_t<Ts...>::value;
    template <typename... Ts>             constexpr inline bool all_different = all_different_t<Ts...>::value;


    template <typename From, typename To> using convert_to = To;


    /** Asserts the elements of a given tuple can be used to initialize an object of type T. */
    template <typename Tuple, typename T> concept constructor_tuple_for = requires (Tuple tpl) {
        std::apply([] (auto&&... args) { T { fwd(args)... }; }, fwd(tpl));
    };
}