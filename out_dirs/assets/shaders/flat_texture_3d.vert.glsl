#version 430

uniform mat4 camera;

in vec3 position;
in vec2 uv;

out vec2 frag_uv;


void main() {
    gl_Position = (camera * vec4(position, 1));
    frag_uv = uv;
}
