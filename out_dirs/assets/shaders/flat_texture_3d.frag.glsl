#version 430

in vec2 frag_uv;
uniform sampler2D tile_texture;

out vec4 color;


void main() {
    color = texture(tile_texture, frag_uv);
}