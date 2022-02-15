#pragma once

#include <chrono>


namespace ve {
    namespace defs {
        using namespace std::chrono;
        
        
        template <typename TimePoint, typename Duration = nanoseconds>
        inline Duration time_since(const TimePoint& when) {
            return duration_cast<Duration>(TimePoint::clock::now() - when);
        }


        template <typename TimePoint>
        inline TimePoint epoch_time(void) {
            return TimePoint { };
        }
    }
    
    using namespace defs;
}