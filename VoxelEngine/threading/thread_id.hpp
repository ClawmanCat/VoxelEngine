#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/immovable.hpp>

#include <atomic>


namespace ve {
    // The thread counter is used generate unique IDs between threads without having to do atomic writes for every ID.
    // To generate the unique ID, thread_id * thread_limit + threadlocal_id can be calculated instead.
    // This only requires us to do an atomic write once when the thread is created.
    // Assuming thread_limit = 1 << 24, this gives ~10^12 unique IDs per thread and ~16M threads.
    class thread_counter {
    public:
        constexpr static inline u64 thread_limit = 1 << 24; // ~16M
        constexpr static inline u64 null_thread  = thread_limit + 1; // Guaranteed to not be used by a thread.
        
        
        [[nodiscard]] static u64 get_thread_id(void) noexcept {
            return thread_id;
        }
        
    private:
        thread_counter(void) = delete;
    
    
        [[nodiscard]] static u64 assign_id(void) {
            u64 id = counter++;
            VE_ASSERT(id < thread_limit);
        
            return id;
        }
        
        
        static inline std::atomic<u64> counter   = 0;
        thread_local static inline u64 thread_id = assign_id();
    };
}