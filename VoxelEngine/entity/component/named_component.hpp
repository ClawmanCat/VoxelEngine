#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/string_arg.hpp>


namespace ve {
    // Wraps an object of type T and a name, such that all objects of the same type with the same name
    // have the same type, but objects of the same type with a different name have different types.
    template <typename T, meta::string_arg Name>
    struct named_component {
        T value;
    };
}