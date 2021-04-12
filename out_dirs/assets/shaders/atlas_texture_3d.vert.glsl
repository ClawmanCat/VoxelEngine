#version 430

uniform mat4 camera;
uniform float near;
uniform mat4 transform = mat4(1.0);

in vec3 position;
in vec2 uv;
in uint tex_index;

out vec2 frag_uv;
out float frag_log_z;
out flat uint frag_tex_index;


const float f_coef = 2.0f / log2(1e9f + 1.0f);


void main() {
    gl_Position = transform * vec4(position, 1.0f);
    gl_Position = camera * gl_Position;
    gl_Position.z = log2(max(near, 1.0f + gl_Position.w)) * f_coef - 1.0f;

    frag_uv = uv;
    frag_log_z = 1.0f + gl_Position.w;
    frag_tex_index = tex_index;
}
