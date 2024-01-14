#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/meta/value.hpp>
#include <VoxelEngine/utility/meta/logical.hpp>

#include <variant>


namespace ve::meta {
    /**
     * In a template-foreach method, indicates the loop should return early (i.e. equivalent to 'break' if Payload == void, or 'return payload' otherwise).
     * To control the breaking behaviour with a runtime value, stop_token.stop can be set to false to disable a given token.
     */
    template <typename Payload> struct stop_token {
        using value_type = Payload;

        Payload payload;
        bool stop = true;
    };


    /** @copydoc stop_token */
    template <> struct stop_token<void> {
        using value_type = void;

        bool stop = true;
    };


    namespace detail {
        template <typename T> using to_value_type = typename T::value_type;


        template <std::size_t Start, std::size_t Count, typename F, typename... Vs>
        constexpr inline std::conditional_t<(sizeof...(Vs) == 0), void, std::variant<Vs..., null_type>> invoke_stop_token_fn(F&& fn) {
            if constexpr (Start == Count) {
                if constexpr (sizeof...(Vs) == 0) return;
                else return std::variant<Vs..., null_type> { null_type {} };
            } else {
                using invoke_result_type = std::invoke_result_t<F, value<Start>>;


                if constexpr (is_template_v<stop_token, invoke_result_type>) {
                    auto token = std::invoke(fn, value<Start>{});

                    if constexpr (std::is_void_v<typename decltype(token)::value_type>) {
                        if (token.stop) return std::variant<Vs..., null_type> { null_type {} };
                    } else {
                        if (token.stop) {
                            return std::variant<Vs..., null_type> { std::move(token.payload) };
                        }
                    }

                    if constexpr (Start + 1 < Count) {
                        return invoke_stop_token_fn<(Start + 1), Count, F, Vs...>(fwd(fn));
                    } else {
                        return std::variant<Vs..., null_type> { null_type {} };
                    }
                } else {
                    std::invoke(fn, value<Start>{});
                    return invoke_stop_token_fn<(Start +1), Count, F, Vs...>(fwd(fn));
                }
            }
        }
    }


    /**
     * Invokes the given function as fn(meta::value<I>) for every I in the range [Start, Start + Count].
     * If fn returns stop_token<NonVoid> for any index, the return type of this function is std::variant<R..., null_type>,
     * where R... are all possibly-returned token types.
     * If none of the returned tokens have .stop = true, the variant is returned with a value of type null_type.
     * If none of the invocations of fn return a stop token the return type of this method is void.
     * @tparam P The class template ve::meta::pack. This must be passed as a template parameter to prevent circular dependencies.
     * @tparam Start The first index for which fn is invoked.
     * @tparam Count The number of indices after Start for which fn is invoked.
     * @param fn A function invocable as fn(meta::value<I>) where I is of type std::size_t, returning either void or meta::stop_token.
     * @return The return type of this function is dependent on fn:
     *  - If all invocations of fn return void, this function returns void.
     *  - If at least one invocation of fn returns meta::stop_token<T> where T != void, the return type of this function is std::variant<R..., null_type>,
     *    where R... are all value_types of stop tokens returned by fn.
     *  If this function returns a variant, it will contain the value of the first stop token for which .stop was true, or null_type if no such token existed.
     *
     *  Example:
     *  ~~~
     *  template <typename... Ts> struct pack {
     *      template <std::size_t N> using get = ...;
     *
     *      template <typename F> auto foreach_indexed(F&& fn) {
     *          return meta::invoke_stop_token_fn<pack, 0, sizeof...(Ts)>([&] <std::size_t I> (meta::value<I>) {
     *              return fn.template operator()<get<I>, I>();
     *          });
     *      }
     *  };
     *
     *
     *  using P = pack<int, bool, std::string>;
     *  std::tuple<int, bool, std::string> tpl;
     *
     *  // Returns void since no invocation of fn returns a stop token.
     *  P::foreach_indexed([&] <typename T, std::size_t I> { std::get<I>(tpl) = ...; });
     *
     *  // Returns std::variant<int*, bool*, std::string*, null_type> with either the payload of the first token for which .stop = true, or null_type.
     *  P::foreach_indexed([&] <typename T, std::size_t I> {
     *      auto& value = std::get<I>(tpl);
     *      return meta::stop_token<T*> { .payload = &value, .stop = some_condition(value) };
     *  });
     *  ~~~
     */
    template <template <typename...> typename P, std::size_t Start, std::size_t Count, typename F> constexpr inline auto invoke_stop_token_fn(F&& fn) {
        return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            // Get the result type of invoking F for each element, either void or stop_token<T>.
            using variants = typename P<
                std::invoke_result_t<F, meta::value<Start + Is>>...
            >
                ::template filter_trait<negation<std::is_void>::template type>      // Remove all functions returning void
                ::template map<detail::to_value_type>                               // For each stop token get its value type
                ::template filter_trait<negation<std::is_void>::template type>      // Remove all void stop tokens
                ::template unique<>;                                                // Remove any duplicates.


            return variants::apply([&] <typename... Vs> {
                return detail::invoke_stop_token_fn<Start, Count, F, Vs...>(fwd(fn));
            });
        } (std::make_index_sequence<Count>());
    }


    /** Equivalent to invoke_stop_token_fn, but invokes fn(meta::type<V>, meta::value<I>) for every V in Vs, rather than just using meta::value<I> for every index. */
    template <template <typename...> typename P, typename... Vs> constexpr inline auto invoke_stop_token_fn_for_range(auto&& fn) {
        return invoke_stop_token_fn<P, 0, sizeof...(Vs)>([&] <std::size_t I> (meta::value<I> index) {
            return std::invoke(fn, meta::type<typename P<Vs...>::template nth<I>>{}, index);
        });
    }
}