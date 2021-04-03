#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side_controller.hpp>


namespace ve {
    class client : public side_controller<side::CLIENT> {};
}