#pragma once

#include <VoxelEngine/core/core.hpp>

#include <exception>


namespace ve {
    inline void on_fatal_error(const std::string& msg = "No information available.") {
        throw std::runtime_error(msg);
    }
}