#version 430

#include "utility/pbr.util.glsl"
#include "structs/light.util.glsl"
#include "structs/bloom.util.glsl"
#include "structs/vertex.util.glsl"


layout (std140, binding = 2) uniform U_Lighting { LightingData light_data; };

#ifndef NO_BLOOM
    layout (std140, binding = 3) uniform U_BloomData { BloomData bloom_data; };
    uniform sampler2D l_bloom;
#endif

uniform sampler2D l_position;
uniform sampler2D l_color;


in NO_VERTEX_BLOCK vertex;
out vec4 color;


void main() {
    // Apply Bloom.
    #ifndef NO_BLOOM
        vec3 bloom = texture(l_bloom, vertex.uv).rgb * bloom_data.bloom_intensity;
    #else
        vec3 bloom = vec3(0.0, 0.0, 0.0);
    #endif

    color = texture(l_color, vertex.uv);
    color.rgb += bloom;


    // Perform tone mapping and gamma correction.
    color = linear_to_SRGB(tone_map(color, light_data.exposure));
    gl_FragDepth = texture(l_position, vertex.uv).w;
}
