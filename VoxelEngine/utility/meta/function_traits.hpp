#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


namespace ve::meta {
    template <typename T> struct function_traits {
        constexpr static inline bool is_callable              = false;
        constexpr static inline bool is_function_pointer      = false;
        constexpr static inline bool is_member_variable       = false;
        constexpr static inline bool is_member_function       = false;
        constexpr static inline bool is_const_member_function = false;
    };


    template <typename R, typename... A> struct function_traits<fn<R, A...>> {
        constexpr static inline bool is_callable              = true;
        constexpr static inline bool is_function_pointer      = true;
        constexpr static inline bool is_member_variable       = false;
        constexpr static inline bool is_member_function       = false;
        constexpr static inline bool is_const_member_function = false;

        using return_type      = R;
        using arguments        = meta::pack<A...>;
        using invoke_arguments = meta::pack<A...>;
    };


    template <typename C, typename R, typename... A> struct function_traits<mem_fn<C, R, A...>> {
        constexpr static inline bool is_callable              = true;
        constexpr static inline bool is_function_pointer      = true;
        constexpr static inline bool is_member_variable       = false;
        constexpr static inline bool is_member_function       = true;
        constexpr static inline bool is_const_member_function = false;

        using owning_class     = C;
        using return_type      = R;
        using arguments        = meta::pack<A...>;
        using invoke_arguments = meta::pack<C&, A...>;
    };


    template <typename C, typename R, typename... A> struct function_traits<const_mem_fn<C, R, A...>> {
        constexpr static inline bool is_callable              = true;
        constexpr static inline bool is_function_pointer      = true;
        constexpr static inline bool is_member_variable       = false;
        constexpr static inline bool is_member_function       = true;
        constexpr static inline bool is_const_member_function = true;

        using owning_class     = C;
        using return_type      = R;
        using arguments        = meta::pack<A...>;
        using invoke_arguments = meta::pack<const C&, A...>;
    };


    template <typename C, typename T> struct function_traits<mem_var<C, T>> {
        constexpr static inline bool is_callable              = true;
        constexpr static inline bool is_function_pointer      = true; // Debatable, but set as true for consistency with mem_fn.
        constexpr static inline bool is_member_variable       = true;
        constexpr static inline bool is_member_function       = false;
        constexpr static inline bool is_const_member_function = false;

        using owning_class     = C;
        using return_type      = T;
        using arguments        = meta::pack<>;
        using invoke_arguments = meta::pack<C&>;
    };


    template <typename C> requires requires { &C::operator(); } struct function_traits<C> {
        constexpr static inline bool is_callable              = true;
        constexpr static inline bool is_function_pointer      = false;
        constexpr static inline bool is_member_variable       = false;
        constexpr static inline bool is_member_function       = false;
        constexpr static inline bool is_const_member_function = false;

        using owning_class     = C;
        using return_type      = typename function_traits<decltype(&C::operator())>::return_type;
        using arguments        = typename function_traits<decltype(&C::operator())>::arguments;
        using invoke_arguments = typename function_traits<decltype(&C::operator())>::invoke_arguments;
    };



    template <typename T> concept function_pointer                = function_traits<T>::is_function_pointer;
    template <typename T> concept member_variable_pointer         = function_traits<T>::is_member_variable;
    template <typename T> concept member_function_pointer         = function_traits<T>::is_member_function;
    template <typename T> concept const_member_function_pointer   = function_traits<T>::is_const_member_function;
    template <typename T> concept mutable_member_function_pointer = function_traits<T>::is_member_function && !const_member_function_pointer<T>;

    template <typename T, typename C> concept member_variable_pointer_of         = member_variable_pointer<T>         && std::is_same_v<typename function_traits<T>::owning_class, C>;
    template <typename T, typename C> concept member_function_pointer_of         = member_function_pointer<T>         && std::is_same_v<typename function_traits<T>::owning_class, C>;
    template <typename T, typename C> concept const_member_function_pointer_of   = const_member_function_pointer<T>   && std::is_same_v<typename function_traits<T>::owning_class, C>;
    template <typename T, typename C> concept mutable_member_function_pointer_of = mutable_member_function_pointer<T> && std::is_same_v<typename function_traits<T>::owning_class, C>;


    /** Allows deduction of a const-overloaded member function to its const variant. */
    template <typename C, typename R, typename... A> [[nodiscard]] constexpr auto as_const_mem_fn(const_mem_fn<C, R, A...> pointer) {
        return pointer;
    }

    /** Allows deduction of a const-overloaded member function to its mutable variant. */
    template <typename C, typename R, typename... A> [[nodiscard]] constexpr auto as_mutable_mem_fn(mem_fn<C, R, A...> pointer) {
        return pointer;
    }
}