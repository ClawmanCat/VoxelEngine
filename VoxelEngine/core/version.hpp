#pragma once

#include <VoxelEngine/core/typedefs.hpp>

#include <string>


namespace ve {
    struct version {
        std::string state;
        u32 major, minor, patch;
        
        [[nodiscard]] operator std::string(void) const noexcept {
            // TODO: Replace with str() call.
            return state + " " + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        }
    };
}