#version 430

in vec2 frag_uv;
in float frag_log_z;
in flat uint frag_tex_index;

// Should be large enough for the number of textures used.
uniform sampler2D tex[4];

out vec4 color;


const float f_coef = 2.0f / log2(1e9f + 1.0f);


void main() {
    color = texture(tex[frag_tex_index], frag_uv);
    if (color.a == 0.0f) discard;

    gl_FragDepth = log2(frag_log_z) * (0.5f * f_coef);
}