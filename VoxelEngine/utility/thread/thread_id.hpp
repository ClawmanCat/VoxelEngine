#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <atomic>


namespace ve {
    class thread_id {
    public:
        // Limit number of thread IDs for threadsafe_counter.
        constexpr static inline u32 thread_id_limit = (1 << 24);
        
        thread_id(void) = delete;
        
        static u32 get(void) { return id; }
    private:
        static u32 assign(void) {
            u32 next = counter++;
            VE_ASSERT(next < thread_id_limit, "Engine is out of thread IDs to assign.");
            
            return next;
        }
        
        static inline std::atomic<u32> counter = 0;
        static inline thread_local u32 id = assign();
    };
}