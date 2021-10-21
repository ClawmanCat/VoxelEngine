#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx {
    // This struct matches the members used by the struct Light in common.util.glsl.
    struct light_uniform {
        vec3f position;
        vec3f radiance;
        float attenuation;
    };


    // This struct matches the members used by the struct U_Lighting in pbr_single_pass.frag.glsl.
    template <std::size_t LightCount = 32>
    struct lighting_data_uniform {
        std::array<light_uniform, 32> lights;
        u32 num_populated_lights;

        vec3f ambient_light;
    };
}