#pragma once

#include <VoxelEngine/core/core.hpp>


// Default entity type parameter for ECS containers.
#ifndef VE_DEFAULT_ENTITY_TYPE
    #define VE_DEFAULT_ENTITY_TYPE ve::u64
#endif


// Default number of bits left unassigned in each entity ID.
// Default value is 1 to allow for client-side-only entities.
#ifndef VE_DEFAULT_ENTITY_UBITS
    #define VE_DEFAULT_ENTITY_UBITS 1
#endif