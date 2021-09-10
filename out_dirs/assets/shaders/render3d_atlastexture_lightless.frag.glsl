// Basic shader for rendering 3D objects textured from an atlas texture.
// This shader does not perform any lighting calculations.
#version 430


// Uniforms
layout (location = 3) uniform Textures {
    // Note: not all samplers will be populated, only the sampler referenced in the texture index should be used.
    sampler2D samplers[32];
} textures;


// Attributes
in vec2 f_uv;
in float f_log_z;
in flat uint f_tex_index;

out vec4 color;


// Normally the far plane is used to calculate the depth coeficient,
// but since we're using an infinite perspective matrix, a sufficiently large number will have to do.
const float depth_coeficient = 2.0f / log2(1e9f + 1.0f);


void main() {
    color = texture(textures.samplers[f_tex_index], frag_uv);

    // Don't update the depth buffer for transparent vertices.
    // TODO: Handle transparency properly.
    if (color.a == 0.0f) discard;

    gl_FragDepth = log2(f_log_z) * (0.5f * depth_coeficient);
}