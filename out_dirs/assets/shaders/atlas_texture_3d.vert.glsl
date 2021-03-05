#version 430

uniform mat4 camera;
uniform mat4 transform = mat4(1.0);

in vec3 position;
in vec2 uv;
in uint tex_index;

out vec2 frag_uv;
out flat uint frag_tex_index;


void main() {
    gl_Position = transform * vec4(position, 1);
    gl_Position = camera * gl_Position;

    frag_uv = uv;
    frag_tex_index = tex_index;
}
