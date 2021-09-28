// Basic shader for rendering 3D objects textured from an atlas texture.
// This shader does not perform any lighting calculations.
#version 430
#include "common.util.glsl"


// Uniforms
layout (std140) uniform Uniforms {
    Camera camera;
    mat4 transform;
};


// Attributes
in vec3 position;
in vec2 uv;
in uint texture_index;

out vec2 frag_uv;
out float frag_log_z;
out flat uint frag_tex_index;


// Normally the far plane is used to calculate the depth coeficient,
// but since we're using an infinite perspective matrix, a sufficiently large number will have to do.
const float depth_coeficient = 2.0f / log2(1e9f + 1.0f);


void main() {
    gl_Position   = transform * vec4(position, 1.0f);
    gl_Position   = camera.matrix * gl_Position;
    gl_Position.z = log2(max(camera.near, 1.0f + gl_Position.w)) * depth_coeficient - 1.0f;

    frag_uv        = uv;
    frag_log_z     = 1.0f + gl_Position.w;
    frag_tex_index = texture_index;
}