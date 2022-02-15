#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/thread/thread_id.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>


namespace ve {
    template <meta::string_arg ID> struct threadsafe_counter {
    public:
        static u64 next(void) {
            return (thread_id::get() * thread_id::thread_id_limit) + next_id++;
        }
    private:
        static inline thread_local u64 next_id = 0;
    };
}