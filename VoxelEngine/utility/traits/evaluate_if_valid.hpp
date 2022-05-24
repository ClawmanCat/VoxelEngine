#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>


// Only evaluates the condition if it is valid, otherwise returns false.
#define ve_eval_if_valid(...)                                                           \
[] {                                                                                    \
    if constexpr (requires { __VA_ARGS__; }) return __VA_ARGS__;                        \
    else return false;                                                                  \
}()

#define ve_eval_if_valid_c(...)                                                         \
[&] {                                                                                   \
    if constexpr (requires { __VA_ARGS__; }) return __VA_ARGS__;                        \
    else return false;                                                                  \
}()


// Only evaluates the condition if cond is true, otherwise returns false.
#define ve_eval_if_cond(cond, ...)                                                      \
[] {                                                                                    \
    if constexpr (cond) return __VA_ARGS__;                                             \
    else return false;                                                                  \
}()

#define ve_eval_if_cond_c(cond, ...)                                                    \
[&] {                                                                                   \
    if constexpr (cond) return __VA_ARGS__;                                             \
    else return false;                                                                  \
}()


// Evaluates X if cond is true, otherwise evaluates Y.
#define ve_eval_if_else(cond, X, Y)                                                     \
[] {                                                                                    \
    if constexpr (cond) return X;                                                       \
    else return Y;                                                                      \
}()

#define ve_eval_if_else_c(cond, X, Y)                                                   \
[&] {                                                                                   \
    if constexpr (cond) return X;                                                       \
    else return Y;                                                                      \
}()


// Evaluates to the given type if it is valid, otherwise evaluates to null_type.
#define ve_type_if_valid(...)                                                           \
std::remove_reference_t<decltype(*[] {                                                  \
    if constexpr (requires { typename __VA_ARGS__; }) {                                 \
        return (std::add_pointer_t<__VA_ARGS__>) {};                                    \
    }                                                                                   \
                                                                                        \
    else return (ve::meta::null_type*) { };                                             \
} ())>


// Evaluates to the given type if cond is true, otherwise evaluates to null_type.
#define ve_type_if_cond(cond, ...)                                                      \
std::remove_reference_t<decltype(*[] {                                                  \
    if constexpr (cond) {                                                               \
        return (std::add_pointer_t<__VA_ARGS__>) {};                                    \
    }                                                                                   \
                                                                                        \
    else return (ve::meta::null_type*) { };                                             \
} ())>