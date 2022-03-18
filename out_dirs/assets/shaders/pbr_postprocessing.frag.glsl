#version 430
#include "pbr.util.glsl"


#ifndef LIGHT_SIZE_LIMIT
    #define LIGHT_SIZE_LIMIT 128
#endif


layout (std140, binding = 2) uniform U_Lighting {
    Light lights[LIGHT_SIZE_LIMIT];
    uint num_populated_lights;

    vec3 ambient_light;
    float exposure;

    float emissivity_constant;
};

#ifndef NO_BLOOM
    layout (std140, binding = 3) uniform U_BloomData {
        vec3 luma_conversion_weights;
        float bloom_intensity;
        float bloom_threshold;
    };

    uniform sampler2D l_bloom;
#endif

uniform sampler2D l_position;
uniform sampler2D l_color;


in vec2 uv;
out vec4 color;


void main() {
    // Apply Bloom.
    #ifndef NO_BLOOM
        vec3 bloom = texture(l_bloom, uv).rgb * bloom_intensity;
    #else
        vec3 bloom = vec3(0.0, 0.0, 0.0);
    #endif

    color = texture(l_color, uv);
    color.rgb += bloom;


    // Perform tone mapping and gamma correction.
    color = linear_to_SRGB(tone_map(color, exposure));
    gl_FragDepth = texture(l_position, uv).w;
}