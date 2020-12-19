#version 430

in vec2 frag_uv;
uniform sampler2D tex;

out vec4 color;


void main() {
    color = texture(tex, frag_uv);
}