#include <VoxelEngine/utility/thread/thread_pool.hpp>


namespace ve {
    thread_pool& thread_pool::instance(void) {
        static thread_pool i;
        return i;
    }
}