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
// An exception to this rule is the header VoxelEngine/core/game_preinclude_helpers.hpp, which provides facilities to overload aforementioned settings.
#ifdef VE_GAME_PRE_INCLUDE
    #if __has_include(<VE_GAME_PRE_INCLUDE>)
        #include <VE_GAME_PRE_INCLUDE>
    #else
        #warning "A game pre-include file is defined, but it does not exist."
    #endif
#endif