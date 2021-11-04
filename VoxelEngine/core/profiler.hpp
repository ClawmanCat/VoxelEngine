#pragma once

#include <VoxelEngine/core/platform.hpp>

#include <optick.h>


#ifdef VE_DEBUG
    #define VE_PROFILE_BEGIN() OPTICK_START_CAPTURE()
    #define VE_PROFILE_END() OPTICK_STOP_CAPTURE()
    #define VE_PROFILE_SAVE(...) OPTICK_SAVE_CAPTURE(__VA_ARGS__)

    #define VE_PROFILE_FRAME(Name) OPTICK_FRAME(Name)
    #define VE_PROFILE_FN(...) OPTICK_EVENT(__VA_ARGS__)
#else
    #define VE_PROFILE_BEGIN()
    #define VE_PROFILE_END()
    #define VE_PROFILE_SAVE(...)

    #define VE_PROFILE_FRAME(Name)
    #define VE_PROFILE_FN(...)
#endif