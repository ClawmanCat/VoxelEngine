#pragma once

#include <VoxelEngine/core/core.hpp>

#include <mutex>


namespace ve {
    // Equivalent to std::lock_guard, but locks with lock_shared instead of lock.
    template <typename Mutex> class shared_lock_guard {
    public:
        using mutex_type = Mutex;
        
        explicit shared_lock_guard(Mutex& mtx) : mtx(mtx) {
            mtx.lock_shared();
        }
        
        shared_lock_guard(Mutex& mtx, std::adopt_lock_t) : mtx(mtx) {}
    
        ~shared_lock_guard(void) noexcept {
            mtx.unlock_shared();
        }
        
        
        shared_lock_guard(const shared_lock_guard&) = delete;
        shared_lock_guard& operator=(const shared_lock_guard&) = delete;
        
    private:
        Mutex& mtx;
    };
}