// Basic shader for rendering 3D objects textured from an atlas texture.
// This shader does not perform any lighting calculations.
#version 430


// Uniforms
layout (location = 0) uniform Camera {
    mat4 matrix;
    float near;
} camera;

layout (location = 1) uniform Transform {
    mat4 matrix = mat4(1.0);
} transform;


// Attributes
in vec3 position;
in vec2 uv;
in uint texture_index;

out vec2  f_uv;
out float f_log_z;      // log of depth.
out uint  f_tex_index;


// Normally the far plane is used to calculate the depth coeficient,
// but since we're using an infinite perspective matrix, a sufficiently large number will have to do.
const float depth_coeficient = 2.0f / log2(1e9f + 1.0f);

void main() {
    gl_Position   = transform.matrix * vec4(position, 1.0f);
    gl_Position   = camera.matrix * gl_Position;
    gl_Position.z = log2(max(camera.near, 1.0f + gl_Position.w)) * f_coef - 1.0f;

    f_uv        = uv;
    f_log_z     = 1.0f + gl_Position.w;
    f_tex_index = texture_index;
}