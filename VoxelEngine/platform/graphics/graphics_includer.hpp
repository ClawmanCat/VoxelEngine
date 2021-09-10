#pragma once

#include <VoxelEngine/core/core.hpp>


#ifndef VE_GRAPHICS_API
    #error "Please define a graphics API to use. (VE_GRAPHICS_API=opengl or VE_GRAPHICS_API=vulkan)"
#endif


// Usage: #include VE_GFX_HEADER(folder/header.hpp)
#define VE_GFX_HEADER(header) <VoxelEngine/platform/graphics/VE_GRAPHICS_API/header>


namespace ve {
    namespace gfx::VE_GRAPHICS_API { }
    namespace gfxapi = gfx::VE_GRAPHICS_API;
}