#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/maybe_const.hpp>


// Pass templated methods as callables without having to specify the template parameters first.
#define ve_wrap_callable(...) \
[](auto&&... args) { return __VA_ARGS__(fwd(args)...); }

#define ve_wrap_tmpl_callable(...) \
[] <typename... Args> (auto&&... args) { return __VA_ARGS__<Args...>(fwd(args)...); }

#define ve_wrap_ctx_callable(...) \
[&](auto&&... args) { return __VA_ARGS__(fwd(args)...); }


#define ve_get_field(...) [&](auto& obj) -> decltype(auto) { return obj.__VA_ARGS__; }
#define ve_get_fn(...) [&](auto& obj) -> decltype(auto) { return obj.__VA_ARGS__(); }


namespace ve {
    constexpr inline auto no_op    = [](auto&&...) {};
    constexpr inline auto identity = [](auto&& e) { return fwd(e); };
    
    
    template <typename... Fns> struct visitor : public Fns... {
        constexpr explicit visitor(Fns&&... fns) : Fns(fns)... {}
        using Fns::operator()...;
    };
    

    // Returns a function that copies and returns the pre-provided value.
    template <typename T> constexpr inline auto produce(T&& value) {
        return [value = fwd(value)](auto&&...) { return value; };
    }


    // Returns a function that constructs T from its given arguments.
    template <typename T> constexpr inline auto construct(void) {
        return [](auto&&... args) { return T { fwd(args)... }; };
    }


    // Returns a function that casts its argument to the given type.
    template <typename T> constexpr inline auto cast(void) {
        return [](auto&& arg) { return static_cast<T>(arg); };
    }


    // Convert between 2-element tuples and pairs.
    template <typename A, typename B> inline std::pair<A, B> to_pair(std::tuple<A, B>&& tpl) {
        return std::make_pair(std::move(std::get<0>(tpl)), std::move(std::get<1>(tpl)));
    }

    template <typename A, typename B> inline std::tuple<A, B> to_tuple(std::pair<A, B>&& pair) {
        return std::make_tuple(std::move(std::get<0>(pair)), std::move(std::get<1>(pair)));
    }
    
    
    template <typename Cls, typename T> constexpr inline auto get_field(mem_var<Cls, T> member) {
        return [member = member](meta::maybe_const<Cls> auto& object) { return object.*member; };
    }
    
    
    // Returns a function equal to the provided function, but returning void.
    template <typename Fn> constexpr inline auto discard_result(Fn&& fn) {
        return [fn = fwd(fn)] (auto&&... args) { std::invoke(fn, fwd(args)...); };
    }
    

    // Returns a function that checks if the given field is equal to the given value.
    template <typename Cls, typename T> constexpr inline auto equal_on(mem_var<Cls, T> member, auto&& val) {
        return [val = fwd(val), member = member](const Cls& o) { return o.*member == val; };
    }


    // Returns a function that checks if invoking the given member with the given arguments returns a result equal to the given value.
    template <typename Cls, typename T, typename... Args>
    constexpr inline auto equal_on(mem_fn<Cls, T, Args...> member, auto&& val, Args&&... args) {
        return [val = fwd(val), member = member, ...args = fwd(args)](const Cls& o) { return (o.*member)(args...) == val; };
    }


    template <typename Cls, typename T, typename... Args>
    constexpr inline auto equal_on(const_mem_fn<Cls, T, Args...> member, auto&& val, Args&&... args) {
        return [val = fwd(val), member = member, ...args = fwd(args)](const Cls& o) { return (o.*member)(args...) == val; };
    }

    
    // Helpers for ve::bind
    namespace detail {
        template <std::size_t N, std::size_t... Ns> struct num_before {
            constexpr static std::size_t value = 0;
        };
        
        template <std::size_t N, std::size_t N0, std::size_t... Ns> struct num_before<N, N0, Ns...> {
            constexpr static std::size_t value = std::size_t(N > N0) + num_before<N, Ns...>::value;
        };
        
        template <std::size_t N, std::size_t... Ns> constexpr bool contains = ((N == Ns) || ...);
        
        
        template <std::size_t N, typename H, typename... T>
        constexpr decltype(auto) get_nth(H&& h, T&&... t) {
            if constexpr (N == 0) return fwd(h);
            else return get_nth<(N - 1), T...>(fwd(t)...);
        }
    }
    
    
    template <std::size_t... Indices, typename Fn, typename... BArgs>
    constexpr auto bind(Fn&& fn, BArgs&&... bound_args) {
        return [fn = fwd(fn), ...bound_args = fwd(bound_args)] <typename... RArgs> (RArgs&&... rest_args) mutable -> decltype(auto) {
            return [&] <std::size_t... Is> (std::index_sequence<Is...>) -> decltype(auto) {
                return std::invoke(fn, [&] <std::size_t I = Is> () -> decltype(auto) {
                    if constexpr (detail::contains<I, Indices...>) {
                        constexpr std::size_t index = detail::num_before<I, Indices...>::value;
                        return detail::get_nth<index>(bound_args...);
                    } else {
                        constexpr std::size_t index = I - detail::num_before<I, Indices...>::value;
                        return detail::get_nth<index>(fwd(rest_args)...);
                    }
                }.template operator()<Is>()...);
            }(std::make_index_sequence<sizeof...(BArgs) + sizeof...(RArgs)>());
        };
    }
    
    
    template <typename Fn, typename... BArgs>
    constexpr inline auto bind_front(Fn&& fn, BArgs&&... bound_args) {
        return [fn = fwd(fn), ...bound_args = fwd(bound_args)] (auto&&... rest_args) mutable -> decltype(auto) {
            return std::invoke(fn, BArgs { bound_args }..., fwd(rest_args)...);
        };
    }
    
    
    template <typename Fn, typename... BArgs>
    constexpr inline auto bind_back(Fn&& fn, BArgs&&... bound_args) {
        return [fn = fwd(fn), ...bound_args = fwd(bound_args)] (auto&&... rest_args) mutable -> decltype(auto) {
            return std::invoke(fn, fwd(rest_args)..., BArgs { bound_args }...);
        };
    }
}