#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/value.hpp>

#include <functional>


/**
 * Executes the provided callable on program startup.
 * @param Identifier An unique identifier associated with this VE_ON_PROGRAM_START invocation.
 * @param __VA_ARGS__ The callable to invoke on program startup.
 */
#define VE_ON_PROGRAM_START(Identifier, ...)                \
struct Identifier {                                         \
    static inline ve::meta::null_type initializer = [] {    \
        std::invoke(__VA_ARGS__);                           \
        return ve::meta::null_type { };                     \
    } ();                                                   \
}