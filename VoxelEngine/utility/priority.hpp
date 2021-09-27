#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Note: any u16 is a valid priority,
    // these names exist purely to simplify ordering when only a few priority levels are needed.
    namespace priority {
        constexpr inline u16 LOWEST  = -10'000;
        constexpr inline u16 LOW     = -1'000;
        constexpr inline u16 NORMAL  = +0;
        constexpr inline u16 HIGH    = +1'000;
        constexpr inline u16 HIGHEST = +10'000;
    }


    constexpr inline std::array priorities = {
        priority::LOWEST,
        priority::LOW,
        priority::NORMAL,
        priority::HIGH,
        priority::HIGHEST
    };
}