// Basic shader for rendering 3D objects textured from an atlas texture.
// This shader does not perform any lighting calculations.
#version 430


// Uniforms
uniform int k;
uniform sampler2D textures[32];


// Attributes
in vec2 frag_uv;
in float frag_log_z;
in flat uint frag_tex_index;

out vec4 color;


// Normally the far plane is used to calculate the depth coeficient,
// but since we're using an infinite perspective matrix, a sufficiently large number will have to do.
const float depth_coeficient = 2.0f / log2(1e9f + 1.0f);


void main() {
    color = texture(textures[frag_tex_index], frag_uv);
    if (color.a == 0.0f) discard;

    gl_FragDepth = log2(frag_log_z) * (0.5f * depth_coeficient);
}