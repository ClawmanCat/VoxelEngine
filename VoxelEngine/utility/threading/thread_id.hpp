#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/debug/assert.hpp>

#include <atomic>


namespace ve {
    class thread_id {
    public:
        constexpr static inline u32 thread_id_limit = (1 << 24);

        [[nodiscard]] static u32 this_thread_id(void) { return current_thread_id; }
        [[nodiscard]] static u32 main_thread_id(void) { return 0;                 }

        [[nodiscard]] static bool is_main_thread(void) { return this_thread_id() == main_thread_id(); }
    private:
        [[nodiscard]] static u32 assign(void) {
            VE_ASSERT(counter != thread_id_limit, "Cannot generate new thread ID: thread limit exceeded.");
            return counter++;
        }


        static std::atomic_uint32_t counter;
        static thread_local const u32 current_thread_id;
    };
}