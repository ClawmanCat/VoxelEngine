#pragma once

#include <VoxelEngine/core/core.hpp>


// Utility for inline chaining of operations on objects.
// Usage example:
// some_fn(
//      MyClass{}
//          | then([] (auto& o) { o.set_x(4);  })
//          | then([] (auto& o) { o.set_y(11); })
// );
namespace ve {
    template <typename Fn> struct then_t {
        Fn fn;

        constexpr decltype(auto) operator()(auto& arg) const {
            std::invoke(fn, fwd(arg));
        }
    };

    constexpr auto then(auto fn) { return then_t<decltype(fn)> { std::move(fn) }; }


    template <typename T, typename Fn>
    constexpr T&& operator|(T&& lhs, then_t<Fn> rhs) {
        std::invoke(rhs, (T&) lhs);
        return fwd(lhs);
    }
}