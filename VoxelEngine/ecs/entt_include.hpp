#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::detail {
    // This just calls VE_ASSERT. For some reason calling it from the header leads to it not being able to see
    // ve::cat, even if it is included here directly.
    extern void entt_assert_override(bool cond, const char* msg);
}

#ifdef VE_DEBUG
    #define ENTT_ASSERT(...) ve::detail::entt_assert_override(__VA_ARGS__)
#else
    #define ENTT_DISABLE_ASSERT
#endif


#include <entt/entt.hpp>