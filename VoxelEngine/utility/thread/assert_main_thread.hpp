#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <thread>


namespace ve {
    namespace detail {
        const inline auto main_thread_id = std::this_thread::get_id();
    }


    // Assert the caller is invoking this method on the main thread.
    inline void assert_main_thread(void) {
        VE_DEBUG_ASSERT(
            std::this_thread::get_id() == detail::main_thread_id,
            "This method may only be invoked from the main thread."
        );
    }
}