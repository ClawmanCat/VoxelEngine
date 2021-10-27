#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    class static_entity;


    // Used by static_entities to interface with their associated instance.
    struct self_component { static_entity* self = nullptr; };
}