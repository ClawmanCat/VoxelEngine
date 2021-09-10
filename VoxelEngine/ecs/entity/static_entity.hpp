#pragma once

#include <VoxelEngine/core/core.hpp>

#include <entt/entt.hpp>


namespace ve {
    // While entities need not be associated with any class, it is often useful to create an entity class
    // as a template for many identical entities.
    // By extending this base class, a class may add static components. These components are present on all entities
    // created from that class, and can be accessed as if they were class members.
}