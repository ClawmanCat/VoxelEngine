#version 430
#include "common.util.glsl"
#include "pbr.util.glsl"


layout (std140, binding = 2) uniform U_GlobalLighting {
    vec3 ambient_light;
};


const uint num_samplers = VE_MAX_FS_SAMPLERS;
uniform sampler2D textures[num_samplers];


in vec3 frag_position;
in float frag_log_z;

in flat uint frag_texture_index;
in vec2 frag_uv_color;
in vec2 frag_uv_normal;
in vec2 frag_uv_material;

in flat mat3 TBN;


out vec4 g_position;
out vec4 g_normal;
out vec4 g_color;
out vec4 g_material;


void main() {
    // World position of the fragment, plus its depth as seen from the camera.
    float depth  = log2(frag_log_z) * (0.5 * f_coef);
    g_position   = vec4(frag_position, depth);
    gl_FragDepth = depth;

    // Material data of the fragment (R = roughness, G = metalness, B = ambient occlusion).
    g_material = texture(textures[frag_texture_index], frag_uv_material);
    float occlusion = g_material.b;

    // Color of the fragment, with ambient lighting applied, and in linear color space.
    g_color = SRGB_to_linear(texture(textures[frag_texture_index], frag_uv_color));
    g_color.rgb *= ambient_light * occlusion;
    if (g_color.a == 0.0) discard;

    // Normal of the fragment, with the TBN matrix applied.
    vec3 normal = texture(textures[frag_texture_index], frag_uv_normal).xyz;
    normal = 2.0 * (normal - 0.5); // [0, 1 => -1, 1]
    normal = normalize(TBN * normal);

    g_normal = vec4(normal, 1.0);
}