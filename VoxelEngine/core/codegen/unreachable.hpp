#pragma once

#include <VoxelEngine/core/platform.hpp>


#if defined(VE_COMPILER_CLANG) || defined(VE_COMPILER_GCC)
    #define VE_UNREACHABLE __builtin_unreachable()
#elif defined (VE_COMPILER_MSVC)
    #define VE_UNREACHABLE __assume(0)
#else
    #define VE_UNREACHABLE
#endif