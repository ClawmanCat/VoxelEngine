#version 430

#include "utility/sampler_array.util.glsl"
#include "structs/vertex.util.glsl"


uniform sampler2D textures[SAMPLER_ARRAY_SIZE];

in TEX_VERTEX_BLOCK vertex;
out vec4 color;


// Basic shader for rendering 3D objects textured from an atlas texture.
// This shader does not perform any lighting calculations.
void main() {
    color = sample_array(textures, vertex.texture_index, vertex.uv);
    if (color.a == 0.0f) discard;
}
