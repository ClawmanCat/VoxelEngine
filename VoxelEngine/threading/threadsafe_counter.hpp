#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/threading/thread_id.hpp>


namespace ve {
    template <typename Owner> class threadsafe_counter {
    public:
        [[nodiscard]] static u64 next(void) noexcept {
            return thread_counter::get_thread_id() * thread_counter::thread_limit + next_id++;
        }
        
    private:
        static thread_local inline u64 next_id = 0;
    };
}