#include <VoxelEngine/core/platform.hpp>


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