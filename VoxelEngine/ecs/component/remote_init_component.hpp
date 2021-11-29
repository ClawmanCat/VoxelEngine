#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // The remote init component can be added by a remote instance, and indicates the local instance should initialize
    // the entity according to a known initialization function in system_remote_init.
    // "type" represents a type hash for a type tag uniquely identifying the initializer.
    struct remote_init_component {
        u64 type;
    };


    template <typename T> constexpr inline remote_init_component remote_init_for(void) {
        return remote_init_component { type_hash<T>() };
    }
}