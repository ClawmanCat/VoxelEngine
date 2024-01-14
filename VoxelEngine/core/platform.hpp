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


// Platform
#if defined(_WIN32)
    #define VE_WINDOWS
#elif defined(__linux__)
    #define VE_LINUX
    #define VE_UNIX_LIKE
#elif defined(__APPLE__)
    #define VE_APPLE
    #define VE_UNIX_LIKE
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


// IDE
#if defined(__INTELLISENSE__) || defined(__JETBRAINS_IDE__) || defined(VE_IDE_PASS)
    #define VE_IDE_PASS

    #define VE_IDE_ONLY(...) __VA_ARGS__
    #define VE_NO_IDE_ONLY(...)
#else
    #define VE_IDE_ONLY(...)
    #define VE_NO_IDE_ONLY(...) __VA_ARGS__
#endif