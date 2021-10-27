#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    struct light_component {
        vec3f radiance;
        float attenuation;
    };
}