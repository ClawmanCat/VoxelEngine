#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


namespace ve {
    // The remote init component can be added by a remote instance, and indicates the local instance should initialize
    // the entity according to a known initialization function in system_remote_init.
    struct remote_init_component {
        u64 type;
        std::vector<u8> data;
    };


    template <typename T> inline remote_init_component remote_init_for(const T& value = T()) {
        return remote_init_component { type_hash<T>(), serialize::to_bytes(value) };
    }
}