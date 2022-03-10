#version 430
#include "common.util.glsl"
#include "pbr.util.glsl"


#ifndef LIGHT_SIZE_LIMIT
    #define LIGHT_SIZE_LIMIT 128
#endif


layout (std140, binding = 2) uniform U_Lighting {
    Light lights[LIGHT_SIZE_LIMIT];
    uint num_populated_lights;

    vec3 ambient_light;
    float exposure;
};

layout (std140, binding = 3) uniform U_BloomData {
    vec3 luma_conversion_weights;
    float bloom_intensity;
    float bloom_threshold;
};

uniform sampler2D l_position;
uniform sampler2D l_color;
uniform sampler2D l_bloom;

in vec2 uv;

out vec4 color;


void main() {
    // Apply Bloom.
    color = texture(l_color, uv) + (texture(l_bloom, uv) * bloom_intensity);

    // Perform tone mapping and gamma correction.
    color = linear_to_SRGB(tone_map(color, exposure));
    gl_FragDepth = texture(l_position, uv).w;
}