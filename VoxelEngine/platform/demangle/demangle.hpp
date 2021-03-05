#pragma once

#include <VoxelEngine/core/core.hpp>


#ifdef VE_WINDOWS
    #include <VoxelEngine/platform/demangle/windows.hpp>
#else
    #include <VoxelEngine/platform/demangle/posix.hpp>
#endif