#include <VoxelEngine/utility/threading/thread_id.hpp>


namespace ve {
    std::atomic_uint32_t thread_id::counter = 0;
    thread_local const u32 thread_id::current_thread_id = thread_id::assign();
}