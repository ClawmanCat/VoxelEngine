#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <typename Lock, typename Pred>
    inline bool try_lock_while(Lock& lock, Pred pred) {
        while (pred()) {
            if (lock.try_lock()) return true;
        }
        
        return false;
    }
}