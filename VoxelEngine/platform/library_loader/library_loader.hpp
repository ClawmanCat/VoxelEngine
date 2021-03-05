#pragma once

#include <VoxelEngine/core/core.hpp>


#ifdef VE_WINDOWS
    #include <VoxelEngine/platform/library_loader/windows.hpp>
#else
    #include <VoxelEngine/platform/library_loader/posix.hpp>
#endif