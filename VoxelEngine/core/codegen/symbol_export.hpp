#pragma once

#include <VoxelEngine/core/platform.hpp>

#include <boost/preprocessor.hpp>
#include <boost/vmd/is_number.hpp>


// Exports the associated symbol. Target should be the name of a CMake target.
#ifdef VE_WINDOWS
    #define VE_EXPORT(Target)                                   \
    BOOST_PP_IF(                                                \
        BOOST_VMD_IS_NUMBER(BOOST_PP_CAT(BUILDING_, Target)),   \
        __declspec(dllexport),                                  \
        __declspec(dllimport)                                   \
    )
#else
    #define VE_EXPORT(Target) __attribute__((visibility("default")))
#endif