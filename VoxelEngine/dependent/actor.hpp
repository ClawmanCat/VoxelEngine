#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    using actor_id = u32;
    
    extern actor_id next_actor_id(void);
    
    // Used by the engine and the associated game.
    const static actor_id engine_actor_id  = next_actor_id();
    const static actor_id invalid_actor_id = max_value<actor_id>;
}


#ifdef VE_BUILD_GAME
    #define ve_current_actor ve::engine_actor_id
#elif defined(VE_CURRENT_ACTOR_FN)
    // Plugins may define their own function for providing their actor id.
    #define ve_current_actor VE_CURRENT_ACTOR_FN()
#else
    #define ve_current_actor
    #define ve_no_default_actor
#endif


// For use with default parameters.
#ifdef ve_no_default_actor
    #define ve_default_actor_param(name) name
#else
    #define ve_default_actor_param(name) name = ve_current_actor
#endif