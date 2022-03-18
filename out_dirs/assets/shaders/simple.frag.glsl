// Basic shader for rendering 3D objects textured from an atlas texture.
// This shader does not perform any lighting calculations.
#version 430


// Uniforms
uniform sampler2D textures[VE_MAX_FS_SAMPLERS];


// Attributes
in vec2 frag_uv;
in flat uint frag_tex_index;

out vec4 color;


void main() {
    color = texture(textures[frag_tex_index], frag_uv);
    if (color.a == 0.0f) discard;
}