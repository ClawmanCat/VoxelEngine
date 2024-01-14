#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    /** Constructs a single visitor from multiple callables, each handling some subset of the visitable types. */
    template <typename... Vs> struct visitor : Vs... {
        constexpr explicit visitor(Vs&&... vs) : Vs(fwd(vs))... {}
        using Vs::operator()...;
    };


    template <typename... Vs> [[nodiscard]] constexpr inline visitor<Vs...> make_visitor(Vs&&... vs) {
        return visitor { fwd(vs)... };
    }
}