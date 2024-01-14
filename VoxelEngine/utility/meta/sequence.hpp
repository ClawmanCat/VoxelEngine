#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/value.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/meta/stop_token.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>

#include <utility>
#include <concepts>


namespace ve::meta {
    namespace detail {
        template <std::integral T, T Start, T Stop, T Step, T Current, T... Values>
        consteval inline auto make_integer_sequence_impl(void) {
            constexpr bool is_ascending = (Stop > Start);


            if constexpr (is_ascending) {
                if constexpr (Current >= Stop) return std::integer_sequence<T, Values...>{};
                else return make_integer_sequence_impl<T, Start, Stop, Step, Current + Step, Values..., Current>();
            } else {
                if constexpr (Current <= Stop) return std::integer_sequence<T, Values...>{};
                else return make_integer_sequence_impl<T, Start, Stop, Step, Current + Step, Values..., Current>();
            }
        }


        template <typename T, T... S1, T... S2>
        consteval inline auto join_sequences_impl(std::integer_sequence<T, S1...>, std::integer_sequence<T, S2...>) {
            return std::integer_sequence<T, S1..., S2...>{};
        }


        template <typename S1, typename... Ss>
        consteval inline auto join_all_sequences_impl(void) {
            if constexpr (sizeof...(Ss) == 0) {
                return S1{};
            } else if constexpr (sizeof...(Ss) == 1) {
                return join_sequences_impl(S1{}, Ss{}...);
            } else {
                return join_sequences_impl(S1{}, join_all_sequences_impl<Ss...>());
            }
        }
    }


    /** Type trait to check if a given class is a std::index_sequence. */
    template <typename> struct is_integer_sequence { constexpr static inline bool value = false; };
    /** @copydoc is_integer_sequence */
    template <typename T, T... Vs> struct is_integer_sequence<std::integer_sequence<T, Vs...>> { constexpr static inline bool value = true; };


    /** Concept for @ref is_integer_sequence */
    template <typename T> concept integer_sequence = is_integer_sequence<T>::value;
    /** Concept for @ref is_integer_sequence with a given value_type. */
    template <typename T, typename VT> concept integer_sequence_of = integer_sequence<T> && std::same_as<VT, typename T::value_type>;


    /** Joins the provided std::integer_sequences into a new sequence. */
    template <integer_sequence... Seqs> using join_sequences_t = decltype(detail::join_all_sequences_impl<Seqs...>());

    /** @copydoc join_sequences_t */
    template <integer_sequence... Seqs> consteval inline join_sequences_t<Seqs...> join_sequences(Seqs... seqs) {
        return join_sequences_t<Seqs...>{};
    }


    /**
     * Returns an std::integer_sequence containing every integer between 'Start' and 'Stop' that is a multiple of 'Step' removed from 'Start'.
     * If a signed type is used, a negative step may be used to create a decrementing sequence.
     * E.g.
     *      make_integer_sequence<int, 0, 7, 2>()  returns std::integer_sequence<int, 0, 2, 4, 6>
     *      make_integer_sequence<int, 5, 0, -1>() returns std::integer_sequence<int, 5, 4, 3, 2, 1>
     *
     * @tparam T The integer type used for the sequence.
     * @tparam Start The number to start the sequence at (inclusive).
     * @tparam Stop The number to end the sequence at (exclusive).
     * @tparam Step The step size between numbers in the sequence.
     * @return An std::integer_sequence for the given parameters.
     */
    template <std::integral T, T Start, T Stop, T Step = 1> requires ((Start <= Stop && Step > 0) || (Start >= Stop && Step < 0))
    consteval inline auto make_integer_sequence(void) {
        return detail::make_integer_sequence_impl<T, Start, Stop, Step, Start>();
    }


    /**
     * Invokes fn<I>() for every number in the provided sequence. @ref stop_token can be used with this method.
     * @param seq A std::integer_sequence to iterate over.
     * @param fn A function to invoke for every integer in the sequence.
     * @return The return type of this function depends on that of fn:
     *  - If fn returns meta::stop_token for any value, the return type of this function is std::variant<R..., meta::null_type>,
     *    where R... are all possible stop token value types returned by this function.
     *    The value of this variant is the payload of the first stop token for which stop = true, or null_type if no such token existed.
     *  - In all other cases, the return type of this function is void.
     */
    template <typename F, typename T, T... Values>
    constexpr inline auto foreach_in_sequence(std::integer_sequence<T, Values...> seq, F&& fn) {
        return meta::invoke_stop_token_fn_for_range<meta::pack, meta::value<Values>...>([&] <T Value> (meta::type<meta::value<Value>>, auto) {
            return fn.template operator()<Value>();
        });
    }
}