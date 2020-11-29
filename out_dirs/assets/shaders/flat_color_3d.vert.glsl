#version 430

uniform mat4 camera;

in vec3 position;
in uvec4 color;

out vec4 frag_color;


void main() {
    gl_Position = (camera * vec4(position, 1));
    frag_color = vec4(color) / 255;
}
