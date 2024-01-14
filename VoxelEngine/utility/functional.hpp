#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/function_traits.hpp>

#include <functional>


namespace ve {
    struct no_op_t {
        void operator()(auto&&...) const noexcept {}
    };

    constexpr inline no_op_t no_op;


    /**
     * Given a function pointer F, returns a new function pointer that invokes F and discards the result.
     * Note: F must be a template parameter as capturing it would prevent the resulting lambda from being convertible to a function pointer.
     * @tparam F A function pointer.
     * @return A pointer to a function that invokes F and returns void.
     */
    template <auto F> requires meta::function_traits<decltype(F)>::is_function_pointer
    constexpr inline auto discard(void) {
        return meta::function_traits<decltype(F)>::arguments::apply([] <typename... Args> {
            return +[] (Args&&... args) { return F(fwd(args)...); };
        });
    }


    /** Given a callable 'fn', returns a new callable object that invokes 'fn' and returns void. */
    template <typename F> constexpr inline auto discard(F&& fn) {
        return [f = fwd(fn)] (auto&&... args) { std::invoke(f, fwd(args)...); };
    }


    /**
     * Performs a constexpr-if to select either 'if_true' or 'if_false'.
     * The result of this function can be assigned to a variable of deduced type, allowing for the easy construction of a variable
     * whose type depends on the result of 'Condition', e.g.:
     * ~~~
     * // decltype(value) is 'TypeA' or 'TypeB', depending on 'SomeCondition'.
     * auto value = constexpr_if<SomeCondition>(TypeA { ... }, TypeB { ... });
     * ~~~
     */
    template <bool Condition, typename T, typename F> constexpr inline decltype(auto) constexpr_if(T&& if_true, F&& if_false) {
        if constexpr (Condition) return fwd(if_true);
        else return fwd(if_false);
    }
}