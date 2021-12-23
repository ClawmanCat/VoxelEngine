#pragma once

#include <VoxelEngine/core/platform.hpp>

#include <optick.h>
#include <boost/preprocessor.hpp>


#define VE_IMPL_PROFILER_SAVE_PATH(Path) (Path).string().c_str()


#if defined(VE_PROFILER)
    #define VE_PROFILE_BEGIN() OPTICK_START_CAPTURE()

    #define VE_PROFILE_END(...)                                     \
    OPTICK_STOP_CAPTURE();                                          \
    OPTICK_SAVE_CAPTURE(BOOST_PP_IF(                                \
        BOOST_PP_GREATER(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 0),   \
        VE_IMPL_PROFILER_SAVE_PATH(__VA_ARGS__),                    \
        BOOST_PP_EMPTY()                                            \
    ))

    #define VE_PROFILE_FRAME(Name) OPTICK_FRAME(Name)
    #define VE_PROFILE_FN(...) OPTICK_EVENT(__VA_ARGS__)
    #define VE_PROFILE_WORKER_THREAD(...) OPTICK_THREAD(__VA_ARGS__)
#else
    #define VE_PROFILE_BEGIN()
    #define VE_PROFILE_END(...)

    #define VE_PROFILE_FRAME(Name)
    #define VE_PROFILE_FN(...)
    #define VE_PROFILE_WORKER_THREAD(...)
#endif