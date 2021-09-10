#pragma once

#include <VoxelEngine/core/core.hpp>


// Only evaluates the condition if it is valid, otherwise returns false.
#define ve_eval_if_valid(...)                                            \
[] {                                                                     \
    if constexpr (requires { __VA_ARGS__; }) return __VA_ARGS__;         \
    else return false;                                                   \
}()


// Only evaluates the condition if cond is true, otherwise returns false.
#define ve_eval_if_cond(cond, ...)                                       \
[] {                                                                     \
    if constexpr (cond) return __VA_ARGS__;                              \
    else return false;                                                   \
}()