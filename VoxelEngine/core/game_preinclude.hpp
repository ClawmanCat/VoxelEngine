#pragma once


#ifdef VE_IMPL_CIRCULAR_INCLUDE
    #error                                                                                  \
        "game_preinclude.hpp was included circularly. "                                     \
        "Please make sure you don't include any engine headers in your pre-include file, "  \
        "other than game_preinclude_helpers.hpp."
#endif

#define VE_IMPL_CIRCULAR_INCLUDE


// This header can be used to define a file in game code to include in the engine as part of the core header.
// This can be used to overload different settings structs within engine code.
// Note that the game pre-include file may not include any engine headers itself, as this would produce a circular dependency.
// See DemoGame/preinclude.hpp for an example on how to overload engine settings this way.
#ifdef VE_GAME_PRE_INCLUDE
    #define VE_GAME_PRE_INCLUDE_HEADER <VE_GAME_PRE_INCLUDE>

    #if __has_include(VE_GAME_PRE_INCLUDE_HEADER)
        #include VE_GAME_PRE_INCLUDE_HEADER
    #else
        #error "A game pre-include file is defined, but it does not exist."
    #endif
#endif