#pragma once

#include <VoxelEngine/core/core.hpp>


// Equivalent to __VA_ARGS__ if it is a valid expression, or Fallback otherwise.
// Note: this cannot simply be a function which takes the operation as a templated lambda, as that still results in a compilation failure.
#define VE_EVAL_IF_VALID(Fallback, ...)                                             \
[] () -> decltype(auto) {                                                           \
    if constexpr (requires { __VA_ARGS__; }) return __VA_ARGS__;                    \
    else return Fallback;                                                           \
} ()


#define VE_EVAL_TYPE_IF_VALID(Fallback, ...)                                        \
typename decltype([] {                                                              \
    if constexpr (requires { typename std::type_identity_t<__VA_ARGS__>; }) {       \
        return std::type_identity<__VA_ARGS__>{};                                   \
    } else {                                                                        \
        return std::type_identity<Fallback>{};                                      \
    }                                                                               \
} ())::type