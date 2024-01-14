#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/threading/thread_id.hpp>
#include <VoxelEngine/utility/threading/copyable_atomic.hpp>
#include <VoxelEngine/utility/debug/assert.hpp>

#include <mutex>
#include <atomic>
#include <source_location>


/** Indicates a function may be called concurrently and should be made thread-safe. */
#define VE_CALLED_CONCURRENTLY
/** Indicates a function may not be called concurrently. */
#define VE_DO_NOT_CALL_CONCURRENTLY


namespace ve {
    template <typename T> using atom = copyable_atomic<T>;


    /** Executes the given action while holding the provided lock. */
    template <typename Mutex, typename F, typename... Args>
    inline decltype(auto) with_lock(Mutex& mtx, F&& fn, Args&&... args) {
        std::lock_guard lock { mtx };;
        return std::invoke(fn, fwd(args)...);
    }


    /**
     * Waits on the given condition to become true using the given condition variable.
     * A lock on the provided mutex is automatically acquired before waiting on the condition variable.
     */
    template <typename Mutex, typename CV, typename Cond>
    inline void wait(Mutex& mtx, CV& cv, Cond&& condition) {
        std::unique_lock lock { mtx };
        cv.wait(lock, fwd(condition));
    }


    /** Performs an engine assert that the method was called from the main thread. */
    inline void assert_main_thread(std::source_location current = std::source_location::current()) {
        VE_ASSERT(
            thread_id::is_main_thread(),
            "This method ({} in file {}, line {}) may only be invoked from the main thread.",
            current.function_name(),
            current.file_name(),
            current.line()
        );
    }
}