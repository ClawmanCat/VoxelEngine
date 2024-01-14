#pragma once

#include <VoxelEngine/core/platform.hpp>


// Allows specifying a breakpoint within the code itself, rather than through an IDE.
#ifdef VE_DEBUG
    #ifdef VE_WINDOWS
        #define VE_BREAKPOINT __debugbreak()
    #else
        #include <signal.h>
        #define VE_BREAKPOINT raise(SIGTRAP)
    #endif
#else
    #define VE_BREAKPOINT
#endif


// Platform-specific implementations for the no_unique_address attribute. See https://github.com/llvm/llvm-project/issues/49358
#if defined(VE_COMPILER_MSVC) || (defined(VE_COMPILER_WINCLANG) && __clang_major__ >= 18)
    #define VE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif defined(VE_COMPILER_WINCLANG)
    #define VE_NO_UNIQUE_ADDRESS /* Not Supported */
#else
    #define VE_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif


// Indicates a code path as unreachable, e.g. to mute warnings on unreachable code paths.
#if defined(VE_COMPILER_CLANG) || defined(VE_COMPILER_GCC)
    #define VE_UNREACHABLE __builtin_unreachable()
#elif defined (VE_COMPILER_MSVC)
    #define VE_UNREACHABLE __assume(0)
#else
    #define VE_UNREACHABLE
#endif


// Placeholder for unimplemented features.
#define VE_NOT_YET_IMPLEMENTED \
throw std::runtime_error { std::string { "Method " } + __func__ + " is not yet implemented." };