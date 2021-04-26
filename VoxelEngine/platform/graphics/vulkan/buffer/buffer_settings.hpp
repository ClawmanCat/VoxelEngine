#pragma once

#include <VoxelEngine/core/core.hpp>

#include <vulkan/vulkan.h>


namespace ve::graphics {
    enum class buffer_primitive {
        POINTS, LINES, TRIANGLES, QUADS
    };
    
    
    enum class buffer_storage_mode {
        WRITE_ONCE_READ_ONCE,
        WRITE_ONCE_READ_MANY,
        WRITE_MANY_READ_MANY
    };
}