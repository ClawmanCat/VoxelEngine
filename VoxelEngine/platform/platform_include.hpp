#pragma once

#include <VoxelEngine/core/core.hpp>


// Usage: #include <platform_include(some_header.hpp)>
#if defined(VE_WINDOWS)
    #define platform_include(header) <VoxelEngine/platform/windows/header>
#elif defined(VE_LINUX)
    #define platform_include(header) <VoxelEngine/platform/linux/header>
#elif defined(VE_APPLE)
    #define platform_include(header) <VoxelEngine/platform/osx/header>
#endif