#version 430
#include "common.util.glsl"


layout (std140, binding = 0) uniform U_Camera {
    Camera camera;
};

layout (std140, binding = 1) uniform U_Transform {
    mat4 transform;
};


in vec3 position;
in vec3 normal;
in vec3 tangent;
in uint texture_index;

in vec2 uv_color;
in vec2 uv_normal;
// R = roughness, G = metalness, B = ambient occlusion.
in vec2 uv_material;


out vec3 frag_position;
out float frag_log_z;

out flat uint frag_texture_index;
out vec2 frag_uv_color;
out vec2 frag_uv_normal;
out vec2 frag_uv_material;

out flat mat3 TBN;


void main() {
    frag_position = (transform * vec4(position, 1.0)).xyz;

    // Perform world to screen-space transform.
    // Store log2(depth) to better handle large distances.
    gl_Position   = camera.matrix * vec4(frag_position, 1.0);
    gl_Position.z = log2(max(camera.near, 1.0 + gl_Position.w)) * f_coef - 1.0;
    frag_log_z = 1.0 + gl_Position.w;


    // Passthough UVs and vertex normal.
    frag_texture_index = texture_index;
    frag_uv_color = uv_color;
    frag_uv_normal = uv_normal;
    frag_uv_material = uv_material;


    // Calculate matrix to convert from normal tangent space to world space.
    vec3 bitangent = normalize(cross(normal, tangent));

    vec3 T = normalize((transform * vec4(tangent,   0.0)).xyz);
    vec3 B = normalize((transform * vec4(bitangent, 0.0)).xyz);
    vec3 N = normalize((transform * vec4(normal,    0.0)).xyz);

    TBN = mat3(T, B, N);
}