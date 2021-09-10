#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Wrapper to easily invoke a function that initializes an object by taking it as a parameter.
    template <typename Ret, typename... Args, typename Fn>
    constexpr inline Ret bind_return_value(Fn fn, Args&&... args) {
        Ret result;
        std::invoke(fn, &result, fwd(args)...);
        return result;
    }
}