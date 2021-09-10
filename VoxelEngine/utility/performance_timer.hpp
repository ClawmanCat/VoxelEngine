#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/string.hpp>


namespace ve {
    template <typename Unit = microseconds> class performance_timer {
    public:
        explicit performance_timer(std::string name) : name(std::move(name)), start(steady_clock::now()) {}


        ~performance_timer(void) {
            auto end = steady_clock::now();

            VE_LOG_INFO(cat(
                "Performance timer [", name,
                "] finished in ", duration_cast<Unit>(end - start), "."
            ));
        }
    private:
        std::string name;
        steady_clock::time_point start;
    };
}