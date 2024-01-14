#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/value.hpp>
#include <VoxelEngine/utility/meta/bind.hpp>
#include <VoxelEngine/utility/meta/logical.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/meta/stop_token.hpp>
#include <VoxelEngine/utility/meta/pack/pack_helpers.hpp>

#include <type_traits>
#include <array>
#include <utility>


namespace ve::meta {
    template <typename T> constexpr inline bool is_type_pack = requires { typename T::pack_tag; };
    template <typename T> concept type_pack = is_type_pack<T>;


    /**
     * Utility class which allows storing a variadic pack of types as a typedef and allows for various operations to be done on the associated set of types.
     * @tparam Ts The type pack associated to this pack object.
     */
    template <typename... Ts> struct pack {
        constexpr static inline std::size_t size = sizeof...(Ts);
        constexpr static inline bool empty       = (size == 0);


        /** Used to identify packs for the type_pack concept. */
        using pack_tag = void;


        /** The current pack type. */
        using self = pack<Ts...>;
        /** The first type in the pack, or null_type if the pack is empty. */
        using head = typename detail::pack_splitter<pack, Ts...>::head;
        /** A pack of all but the first type in the pack, or the empty pack if this pack contains either 0 or 1 elements. */
        using tail = typename detail::pack_splitter<pack, Ts...>::tail;

        /** A pack containing the same types as this pack, but in reverse order. */
        using reverse = typename detail::pack_reverser<pack, Ts...>::type;
        /** The last type in the pack, or null_type if the pack is empty. */
        using rhead   = typename reverse::head;
        /** A pack of all but the last type in the pack, or the empty pack if this pack contains either 0 or 1 elements. */
        using rtail   = typename reverse::tail::reverse;


        /** Returns a new pack with the provided types appended to the types of the current pack. */
        template <typename... Xs> using append       = pack<Ts..., Xs...>;
        /** Returns a new pack with the provided types prepended to the types of the current pack. */
        template <typename... Xs> using prepend      = pack<Xs..., Ts...>;
        /** Equivalent to append, but accepts a pack of types rather than a list of types. */
        template <typename Pack>  using append_pack  = typename Pack::template prepend<Ts...>;
        /** Equivalent to prepend, but accepts a pack of types rather than a list of types. */
        template <typename Pack>  using prepend_pack = typename Pack::template append<Ts...>;


        /**
         * Expands the contents of this pack into the provided class template.
         * E.g. pack<X, Y, Z>::expand_inside<std::tuple> -> std::tuple<X, Y, Z>
         */
        template <template <typename...> typename F> using expand_inside  = F<Ts...>;

        /**
         * Expands each element of this pack into the provided class template and stores the results in a new pack.
         * E.g. pack<X, Y, Z>::expand_inside<std::add_const_t> -> pack<const X, const Y, const Z>
         */
        template <template <typename...> typename F> using expand_outside = pack<F<Ts>...>;


        /** Alternative spelling for expand_inside. */
        template <template <typename...> typename F> using to  = expand_inside<F>;
        /** Alternative spelling for expand_outside. */
        template <template <typename...> typename F> using map = expand_outside<F>;


        /** Returns true if this pack contains the given type. */
        template <typename T> constexpr static inline bool contains = (std::is_same_v<T, Ts> || ...);
        /** Returns true if this pack contains all the given types. */
        template <typename... Xs> constexpr static inline bool contains_all = (contains<Xs> && ...);
        /** Returns true if this pack contains any of the given types. */
        template <typename... Xs> constexpr static inline bool contains_any = (contains<Xs> || ...);


        template <std::size_t N> consteval static auto pop_front_impl(void) {
            if constexpr (N >= size) return detail::empointer<pack<>>;
            else if constexpr (N == 0) return detail::empointer<self>;
            else return tail::template pop_front_impl<(N - 1)>();
        }

        /**
         * Removes N types from the front of the pack and returns the result as a new pack.
         * Removing more elements than there are in the pack results in the empty pack.
         */
        template <std::size_t N> using pop_front = detail::depointer<decltype(pop_front_impl<N>())>;


        template <std::size_t N> consteval static auto pop_back_impl(void) {
            if constexpr (N >= size) return detail::empointer<pack<>>;
            else if constexpr (N == 0) return detail::empointer<self>;
            else return rtail::template pop_back_impl<(N - 1)>();
        }

        /**
         * Removes N types from the back of the pack and returns the result as a new pack.
         * Removing more elements than there are in the pack results in the empty pack.
         */
        template <std::size_t N> using pop_back = detail::depointer<decltype(pop_back_impl<N>())>;


        /** Returns the subsequence of the types [Begin, Begin + Count). */
        template <std::size_t Begin, std::size_t Count> requires (Begin <= size && Begin + Count <= size)
        using subrange = typename self
            ::template pop_back<(size - (Begin + Count))>
            ::template pop_front<Begin>;


        /** Returns a pack of the first N elements of the current pack. */
        template <std::size_t N> requires (N <= size) using first_n = pop_back<(size - N)>;
        /** Returns a pack of the last N elements of the current pack. */
        template <std::size_t N> requires (N <= size) using last_n  = pop_front<(size - N)>;


        /** Returns the Nth element of this pack. */
        template <std::size_t N> requires (N < size) using nth = typename pop_front<N>::head;

        
        template <typename T> requires contains<T> consteval static std::size_t find_first_impl(void) {
            if constexpr (std::is_same_v<T, head>) return 0;
            else return tail::template find_first<T> + 1;
        }

        /** Returns the index of the first occurrence of the given type. */
        template <typename T> requires contains<T> constexpr static inline std::size_t find_first = find_first_impl<T>();
        /** Returns the index of the last occurrence of the given type. */
        template <typename T> requires contains<T> constexpr static inline std::size_t find_last  = (size - 1) - reverse::template find_first<T>;

        
        template <typename T, std::size_t Offset, std::size_t... Previous> consteval static auto find_all_impl(void) {
            if constexpr (empty) return std::array<std::size_t, sizeof...(Previous)> { Previous... };
            else if constexpr (std::is_same_v<head, T>) return tail::template find_all_impl<T, (Offset + 1), Previous..., Offset>();
            else return tail::template find_all_impl<T, (Offset + 1), Previous...>();
        }

        /** Returns an array with the indices of all occurrences of the given type. */
        template <typename T> constexpr static inline auto find_all = find_all_impl<T, 0>();


        template <bool Recursive, typename DelayResolve = void> consteval static auto flatten_impl(void) {
            if constexpr (empty) return detail::empointer<pack<>>;

            else if constexpr (is_type_pack<head>) {
                using flattened_head = std::conditional_t<
                    Recursive,
                    typename head::template flatten<>,
                    head
                >;

                return detail::empointer<
                    typename flattened_head::template append_pack<
                        detail::depointer<decltype(tail::template flatten_impl<Recursive, DelayResolve>())>
                    >
                >;
            }

            else return detail::empointer<
                typename pack<head>::template append_pack<
                    detail::depointer<decltype(tail::template flatten_impl<Recursive, DelayResolve>())>
                >
            >;
        }

        /**
         * Flattens all nested packs within this pack.
         * E.g. pack<X, pack<A, B, C>, Y, Z>::flatten -> pack<X, A, B, C, Y, Z>
         * Note: template parameter is required to prevent issues relating to the deduced return type of flatten_impl.
         */
        template <typename DelayResolve = void> using flatten              = detail::depointer<decltype(flatten_impl<true,  DelayResolve>())>;
        /** Equivalent to @ref flatten, but does not flatten packs within packs. */
        template <typename DelayResolve = void> using flatten_nonrecursive = detail::depointer<decltype(flatten_impl<false, DelayResolve>())>;


        template <typename DelayResolve, typename... Previous> consteval static auto unique_impl(void) {
            if constexpr (empty) return detail::empointer<pack<>>;
            else if constexpr (pack<Previous...>::template contains<head>) return tail::template unique_impl<DelayResolve, Previous...>();
            else {
                using tail_result = detail::depointer<decltype(tail::template unique_impl<DelayResolve, Previous..., head>())>;
                return detail::empointer<typename tail_result::template prepend<head>>;
            }
        }

        /**
         * Returns a pack with all but the first occurrence of every type removed.
         * E.g. pack<X, X, Y, Z, Y>::unique -> pack<X, Y, Z>
         * Note: template parameter is required to prevent issues relating to the deduced return type of unique_impl.
         */
        template <typename DelayResolve = void> using unique = detail::depointer<decltype(unique_impl<DelayResolve>())>;


        /** Returns true if fn<T>() is true for every T in the pack. Returns true for the empty pack. */
        template <typename F> constexpr static bool all(F&& fn) {
            if constexpr (empty) return true;
            else {
                if (!fn.template operator()<head>()) return false;
                else return tail::template all<F>(fwd(fn));
            }
        }


        /** Returns true if fn<T>() is true for any T in the pack. Returns false for the empty pack. */
        template <typename F> constexpr static bool any(F&& fn) {
            if constexpr (empty) return false;
            else {
                if (fn.template operator()<head>()) return true;
                else return tail::template any<F>(fwd(fn));
            }
        }


        /**
         * Invokes fn<T, I>() for every T in the pack with its index I. @ref stop_token can be used with this method.
         * @param fn A function-object that is invocable as fn<T, I>() for every type in the pack T and its index I.
         * @return The return type of this function depends on that of fn:
         *  - If fn returns meta::stop_token for any value, the return type of this function is std::variant<R..., meta::null_type>,
         *    where R... are all possible stop token value types returned by this function.
         *    The value of this variant is the payload of the first stop token for which stop = true, or null_type if no such token existed.
         *  - In all other cases, the return type of this function is void.
         */
        template <typename F, std::size_t I = 0> constexpr static decltype(auto) foreach_indexed(F&& fn) {
            return meta::invoke_stop_token_fn_for_range<pack, Ts...>([&] <typename T, std::size_t I> (meta::type<T>, meta::value<I>) {
                return fn.template operator()<T, I>();
            });
        }


        /**
         * Invokes fn<T>() for every T in the pack. @ref stop_token can be used with this method.
         * @param fn fn A function-object that is invocable as fn<T>() for every type T in the pack.
         * @return If any invocation of fn returns meta::stop_token, returns the payload of that token, or void otherwise.
         */
        template <typename F> constexpr static decltype(auto) foreach(F&& fn) {
            return meta::invoke_stop_token_fn_for_range<pack, Ts...>([&] <typename T, std::size_t I> (meta::type<T>, meta::value<I>) {
                return fn.template operator()<T>();
            });
        }
        
        
        template <typename F> consteval static auto filter_impl(void) {
            if constexpr (empty) return detail::empointer<pack>;
            else if constexpr (F{}.template operator()<head>()) return detail::empointer<typename pack<head>::template append_pack<typename tail::template filter_type<F>>>;
            else return tail::template filter_impl<F>();
        }

        /** Returns a new pack with all types for which F{}<T>() is true. */
        template <typename F>                            using filter_type  = detail::depointer<decltype(filter_impl<F>())>;
        /** Returns a new pack with all types for which F<T>() is true. */
        template <auto F>                                using filter_value = filter_type<detail::predicate_wrapper<F>>;
        /** Returns a new pack with all types for which Trait<T>::value is true. */
        template <template <typename...> typename Trait> using filter_trait = filter_type<detail::trait_wrapper<Trait>>;


        /** Invokes fn<Ts...>(). */
        template <typename F>
        constexpr static decltype(auto) apply(F&& fn) { return fn.template operator()<Ts...>(); }


        /** Removes the Nth element from the pack. */
        template <std::size_t N> requires (N < size) using erase = typename subrange<0, N>
            ::template append_pack<subrange<(N + 1), (size - N - 1)>>;

        
        template <typename T> consteval static auto erase_first_impl(void) {
            if constexpr (contains<T>) return erase<find_first<T>> {};
            else return self { };
        }

        /** Removes the first occurrence of T from the pack. */
        template <typename T> using erase_first = decltype(erase_first_impl<T>());
        /** Removes all occurrences of T from the pack. */
        template <typename T> using erase_all = filter_trait<negation<bind<std::is_same, T>::template front>::template type>;
    };
}