#pragma once

#include <VoxelEngine/core/core.hpp>

#include <thread>
#include <string_view>
#include <expected>


namespace ve {
    enum class thread_priority : u32 {
        BACKGROUND,
        LOWEST,
        LOW,
        NORMAL,
        HIGH,
        HIGHEST,
        TIME_CRITICAL
    };


    /** Sets the name of the thread with the given ID. Returns void or an error message if the operation failed. */
    extern std::expected<void, std::string> set_thread_name(std::thread::native_handle_type id, std::string_view name);
    /** Sets the priority of the thread with the given ID. Returns void or an error message if the operation failed. */
    extern std::expected<void, std::string> set_thread_priority(std::thread::native_handle_type id, thread_priority priority);
}