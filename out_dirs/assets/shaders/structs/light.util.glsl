#pragma once


#ifndef LIGHT_SIZE_LIMIT
    #define LIGHT_SIZE_LIMIT 128
#endif


struct Light {
    vec3 position;
    vec3 radiance;
    float attenuation;
};


struct LightingData {
    Light lights[LIGHT_SIZE_LIMIT];
    uint num_populated_lights;

    vec3 ambient_light;
    float exposure;
    float emissivity_constant;
};
