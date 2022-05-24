#version 430

#include "utility/math.util.glsl"
#include "utility/pbr.util.glsl"
#include "utility/sampler_array.util.glsl"
#include "structs/vertex.util.glsl"


uniform sampler2D textures[SAMPLER_ARRAY_SIZE];


in PBR_VERTEX_BLOCK vertex;
in flat mat3 TBN;

out vec4 g_position;
out vec4 g_normal;
out vec4 g_color;
out vec4 g_material;


void main() {
    // World position of the fragment and its normalized depth.
    g_position = vec4(vertex.position, gl_FragCoord.z);

    // Material data of the fragment (R = roughness, G = metalness, B = ambient occlusion, A = emissivity).
    g_material = sample_array(textures, vertex.texture_index, vertex.uv_material);

    // Color of the fragment, converted to linear color space.
    g_color = SRGB_to_linear(sample_array(textures, vertex.texture_index, vertex.uv_color));
    if (g_color.a == 0.0) discard;

    // Normal of the fragment, with the TBN matrix applied.
    vec3 normal = sample_array(textures, vertex.texture_index, vertex.uv_normal).xyz;
    normal = 2.0 * (normal - 0.5); // [0, 1 => -1, 1]
    normal = normalize(TBN * normal);

    g_normal = vec4(normal, 1.0);
}
