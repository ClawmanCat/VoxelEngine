#pragma once

#include <VoxelEngine/core/core.hpp>

#include <optional>
#include <thread>


namespace ve::ecs::schedule {
    /** Information about a given worker in the scheduler. */
    struct worker_info {
        // Thread is nullopt in the case of the main thread worker.
        std::optional<std::thread> thread;
        std::size_t worker_index;


        [[nodiscard]] bool is_main_thread(void) const { return !thread.has_value(); }
    };
}