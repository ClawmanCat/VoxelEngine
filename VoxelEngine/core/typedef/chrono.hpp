#pragma once

#include <chrono>


namespace ve {
    namespace defs {
        using namespace std::chrono;
        
        
        template <typename TimePoint, typename Duration = nanoseconds>
        inline Duration time_since(const TimePoint& when) {
            return duration_cast<Duration>(TimePoint::clock::now() - when);
        }
    }
    
    using namespace defs;
}