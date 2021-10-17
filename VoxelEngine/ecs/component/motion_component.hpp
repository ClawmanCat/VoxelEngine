#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    struct motion_component {
        vec3f linear_velocity  = vec3f { 0 };
        quatf angular_velocity = glm::identity<quatf>();
    };
}