#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    class static_entity;


    // Used by static_entities to interface with their associated instance.
    struct self_component {
        using non_syncable_tag  = void;
        using non_removable_tag = void;

        static_entity* self = nullptr;
    };
}