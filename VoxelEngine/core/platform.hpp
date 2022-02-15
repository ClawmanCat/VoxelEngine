#pragma once


// Build Configuration
#if defined(NDEBUG) && !defined(VE_ENABLE_DEBUGGING)
    #define VE_RELEASE

    #define VE_DEBUG_ONLY(...)
    #define VE_RELEASE_ONLY(...) __VA_ARGS__
#else
    #define VE_DEBUG
    
    #define VE_DEBUG_ONLY(...) __VA_ARGS__
    #define VE_RELEASE_ONLY(...)
#endif


#ifdef VE_DEBUG
    #define VE_IF_DEBUG_ELSE(If, Else) If
#else
    #define VE_IF_DEBUG_ELSE(If, Else) Else
#endif


// Platform
#if defined(_WIN32)
    #define VE_WINDOWS
#elif defined(__linux__)
    #define VE_LINUX
#elif defined(__APPLE__)
    #define VE_APPLE
#endif


// Compiler
#if defined(_MSC_VER) && defined(__clang__)
    #define VE_COMPILER_CLANG
    #define VE_COMPILER_WINCLANG
#elif defined(_MSC_VER)
    #define VE_COMPILER_MSVC
#elif defined(__clang__)
    #define VE_COMPILER_CLANG
#elif defined(__GNUC__)
    #define VE_COMPILER_GCC
#else
    #error "Unsupported compiler detected! Please use MSVC, Clang or GCC."
#endif