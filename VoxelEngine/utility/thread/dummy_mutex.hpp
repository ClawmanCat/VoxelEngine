#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    struct dummy_mutex {
        void lock(void) {}
        void unlock(void) {}
        bool try_lock(void) { return true; }
    };
}