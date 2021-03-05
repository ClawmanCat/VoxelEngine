#pragma once

#include <VoxelEngine/core/core.hpp>

#include <array>


namespace ve {
    enum class priority : i16 {
        LOWEST  = -10'000,
        LOW     = -1'000,
        NORMAL  = +0,
        HIGH    = +1'000,
        HIGHEST = +10'000
    };
}