#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Note: locks use RAII, so nothing has to be acquired here explicitly,
    // the lock will be locked for the duration of the function call.
    template <typename Pred, typename Lock>
    inline decltype(auto) do_locked(const Lock& lock, Pred pred) {
        return pred();
    }
}