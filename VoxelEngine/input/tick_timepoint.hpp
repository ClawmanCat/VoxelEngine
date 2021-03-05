#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    struct tick_timepoint {
        u64 tick = 0;
        steady_clock::time_point time = steady_clock::now();
    };
}