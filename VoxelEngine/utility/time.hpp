#pragma once

#include <VoxelEngine/core/core.hpp>

#include <chrono>
#include <utility>
#include <optional>


namespace ve::time {
    using namespace std::chrono_literals;

    using clock     = std::chrono::steady_clock;
    using timestamp = typename clock::time_point;
    using duration  = typename clock::duration;
    using interval  = std::pair<timestamp, timestamp>;


    constexpr inline timestamp epoch(void);

    struct tick_timestamp {
        timestamp when = epoch();
        u64 tick = 0;

        VE_COMPARE_AS(tick);
    };


    constexpr inline bool has_temporal_intersection(const interval& a, const interval& b) {
        return !(b.first > a.second || a.first > b.second);
    }


    constexpr inline std::optional<interval> temporal_intersection(const interval& a, const interval& b) {
        if (!has_temporal_intersection(a, b)) return std::nullopt;

        return interval {
            std::max(a.first, b.first),
            std::min(a.second, b.second)
        };
    }


    constexpr inline timestamp epoch(void) {
        return timestamp { duration { 0 } };
    }
}