#pragma once

#include <VoxelEngine/core/core.hpp>

#include <string>


namespace ve {
    struct version {
        std::string state;
        u32 major, minor, patch;
        
        [[nodiscard]] operator std::string(void) const noexcept {
            return state + " " + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        }
    };
}