#pragma once


// Configuration
#if defined(NDEBUG)
    #define VE_RELEASE

    #define VE_DEBUG_ONLY(...)
    #define VE_RELEASE_ONLY(...) __VA_ARGS__
#else
    #define VE_DEBUG

    #define VE_DEBUG_ONLY(...) __VA_ARGS__
    #define VE_RELEASE_ONLY(...)
#endif


// System
#if defined(_WIN32)
    #define VE_WINDOWS
#elif defined(__linux__)
    #define VE_LINUX
#elif defined(__APPLE__)
    #define VE_APPLE
#endif