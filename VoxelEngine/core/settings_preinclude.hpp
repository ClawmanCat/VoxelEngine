#pragma once


#ifdef VE_IMPL_CIRCULAR_INCLUDE
    #error                                                                                  \
        "game_preinclude.hpp was included circularly. "                                     \
        "Please make sure you don't include any engine headers in your pre-include file, "  \
        "other than game_preinclude_helpers.hpp."
#endif

#define VE_IMPL_CIRCULAR_INCLUDE


// This header can be used to define settings externally to include in the engine as part of the core header.
// This can be used to overload different settings structs within engine code.
// Note that the pre-include file may not include any engine headers itself, as this would produce a circular dependency.
#ifndef VE_TESTING
    #include <VEEngineSettings/preinclude.hpp>
#endif