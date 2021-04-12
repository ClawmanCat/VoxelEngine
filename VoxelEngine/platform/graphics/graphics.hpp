#pragma once

#include <VoxelEngine/core/core.hpp>


#ifndef VE_GRAPHICS_API
    #error Please define a graphics api to use. (#define VE_GRAPHICS_API opengl or #define VE_GRAPHICS_API vulkan)
#endif

#define VE_IMPL_GRAPHICS_SI_HEADER <VoxelEngine/platform/graphics/VE_GRAPHICS_API/graphics.hpp>
#include VE_IMPL_GRAPHICS_SI_HEADER