#pragma once

#include <VoxelEngine/core/platform.hpp>

#include <cassert>


#if __has_include(<alloca.h>)
    #include <alloca.h>
    
    #define VE_ALLOCA_SUPPORTED
    #define ve_alloca alloca
#elif defined(VE_WINDOWS)
    #include <malloc.h>
    
    #define VE_ALLOCA_SUPPORTED
    #define ve_alloca _alloca
#else
    #define ve_alloca(...) []() -> void* { assert(false); return nullptr; }()
#endif