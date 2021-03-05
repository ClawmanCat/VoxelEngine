#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>

#include <atomic>


namespace ve {
    class thread_counter {
    public:
        constexpr static u64 thread_limit = 1 << 24;
    
    
        thread_counter(void) = delete;
        
        
        [[nodiscard]] static u64 get_thread_id(void) {
            return thread_id;
        }
        
    private:
        [[nodiscard]] static u64 assign_id(void) {
            u64 id = counter++;
            VE_ASSERT(id < thread_limit, "Thread limit exceeded.");
            return id;
        }
        
        
        static inline std::atomic<u64> counter   = 0;
        static inline thread_local u64 thread_id = assign_id();
    };
}