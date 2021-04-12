#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side_controller.hpp>


namespace ve {
    class server : public side_controller<side::SERVER> {};
}