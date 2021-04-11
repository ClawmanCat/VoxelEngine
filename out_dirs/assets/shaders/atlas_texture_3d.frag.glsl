#version 430

in vec2 frag_uv;
in flat uint frag_tex_index;

// Should be large enough for the number of textures used.
uniform sampler2D tex[4];

out vec4 color;


void main() {
    color = texture(tex[frag_tex_index], frag_uv);
    if (color.a == 0.0f) discard;
}