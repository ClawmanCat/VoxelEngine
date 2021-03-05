#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/thread/thread_id.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>


namespace ve {
    // Produces an ID guaranteed to be unique in a multithreaded environment,
    // while not requiring any locking / atomic operations.
    // Passing a different parameter for CounterName will create different counters.
    template <meta::string_arg CounterName> class threadsafe_id {
    public:
        [[nodiscard]] static u64 next(void) noexcept {
            return thread_counter::get_thread_id() * thread_counter::thread_limit + next_id++;
        }
    private:
        static thread_local inline u64 next_id = 0;
    };
}