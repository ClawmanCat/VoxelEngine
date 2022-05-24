#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::detail {
    // Wrapper around VE_ASSERT that can be called by ENTT.
    extern void entt_assert_override(bool cond, const char* msg);
}

#ifdef VE_DEBUG
    #define ENTT_ASSERT(...) ve::detail::entt_assert_override(__VA_ARGS__)
#else
    #define ENTT_DISABLE_ASSERT
#endif


#include <entt/entt.hpp>