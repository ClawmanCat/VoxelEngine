#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    struct dummy_mtx {
        constexpr void lock(void) noexcept {}
        constexpr void unlock(void) noexcept {}
        constexpr bool try_lock(void) noexcept { return true; }
    
        constexpr void lock_shared(void) noexcept {}
        constexpr void unlock_shared(void) noexcept {}
        constexpr bool try_lock_shared(void) noexcept { return true; }
    };
}