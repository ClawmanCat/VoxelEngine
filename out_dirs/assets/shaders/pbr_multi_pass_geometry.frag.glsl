// This is the fragment shader for the first stage in the multipass rendering pipeline.
// It does not perform any shading, but renders fragment data to the GBuffer to be shaded during the next pass.
#version 430
#include "common.util.glsl"


// Coeficient for the logarithmic depth buffer.
// (Normally based on the far plane, but since we have infinite perspective, just use a sufficiently large number.)
const float f_coef = 2.0 / log2(1e9 + 1.0);


layout (std140) uniform U_Lighting {
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
out vec4 position;
out vec4 normal;
out vec4 color;
out vec4 material;


void main() {
    position = vec4(frag_position, log2(frag_log_z) * (0.5 * f_coef));


    // Convert surface normal from local space to world space.
    vec3 tex_normal = texture(textures[frag_tex_index], frag_uv_normal).xyz;
    tex_normal = 2.0 * (tex_normal - 0.5);
    tex_normal = normalize(TBN * tex_normal);

    normal = vec4(tex_normal, 1.0);


    material = texture(textures[frag_tex_index], frag_uv_material);
    float occlusion = material.b;


    color = texture(textures[frag_tex_index], frag_uv_color);

    // Assume the diffuse texture to be in sRGB, and convert to linear before use.
    // Note: it will be converted back after the lighting pass.
    color.rgb = pow(color.rgb, vec3(2.2));
    // Ambient light is applied here, since it does not depend on direction,
    // we don't need to render it from the perspective of the light.
    color.rgb = ambient_light * color.rgb * occlusion;
}