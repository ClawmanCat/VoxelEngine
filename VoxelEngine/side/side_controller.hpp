#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>


namespace ve {
    template <side Side> class side_controller {
    public:
        virtual ~side_controller(void) = default;
    };
}