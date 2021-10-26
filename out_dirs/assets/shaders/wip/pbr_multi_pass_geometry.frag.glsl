// This is the fragment shader for the first stage in the multipass rendering pipeline.
// It does not perform any shading, but renders fragment data to the GBuffer to be shaded during the next pass.
#version 430
#include "common.util.glsl"


// Should be large enough for the number of lights used.
// TODO: Use shader preprocessor to dynamically recompile with actual value.
const int num_lights = 32;

layout (std140) uniform U_Lighting {
    Light lights[num_lights];
    uint num_populated_lights;
    vec3 ambient_light;
};


const uint num_samplers = VE_MAX_FS_SAMPLERS;
uniform sampler2D textures[num_samplers];


in flat uint frag_tex_index;
in vec2 frag_uv_color;
in vec2 frag_uv_normal;
in vec2 frag_uv_material;

in vec3 frag_position;
in float frag_log_z;

in flat mat3 TBN;


// Note: position.z contains the depth component. There is no separate depth buffer.
out vec4 g_position;
out vec4 g_normal;
out vec4 g_color;
out vec4 g_material;


void main() {
    g_position = vec4(frag_position, log2(frag_log_z) * (0.5 * f_coef));


    // Convert surface normal from local space to world space.
    vec3 tex_normal = texture(textures[frag_tex_index], frag_uv_normal).xyz;
    tex_normal = 2.0 * (tex_normal - 0.5);
    tex_normal = normalize(TBN * tex_normal);

    g_normal = vec4(tex_normal, 1.0);


    g_material = texture(textures[frag_tex_index], frag_uv_material);
    float occlusion = g_material.b;


    g_color = texture(textures[frag_tex_index], frag_uv_color);

    // Assume the diffuse texture to be in sRGB, and convert to linear before use.
    // Note: it will be converted back after the lighting pass.
    g_color.rgb = pow(g_color.rgb, vec3(2.2));
    // Ambient light is applied here, since it does not depend on direction,
    // we don't need to render it from the perspective of the light.
    g_color.rgb = ambient_light * g_color.rgb * occlusion;
}