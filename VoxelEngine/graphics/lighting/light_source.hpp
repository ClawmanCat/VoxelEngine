#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx {
    // This struct matches the members used by the struct Light in common.util.glsl.
    struct light_source {
        vec3f position;
        vec3f radiance;
        float attenuation;
    };


    // This struct matches the members used by the struct U_Lighting in pbr_single_pass.frag.glsl.
    template <std::size_t LightCount = 128>
    struct lighting_data {
        constexpr static inline std::size_t light_count_limit = LightCount;

        std::array<light_source, LightCount> lights;
        u32 num_populated_lights;

        vec3f ambient_light;
        float exposure;

        float emissivity_constant;
    };
}