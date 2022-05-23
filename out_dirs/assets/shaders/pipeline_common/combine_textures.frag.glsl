#version 430

#include "utility/sampler_array.util.glsl"
#include "structs/vertex.util.glsl"


// If no combine function is provided, the default behaviour is to add the textures together.
#ifndef COMBINE_TEXTURES_OP
    #define COMBINE_TEXTURES_OP vec4 combine(vec4 accumulated, vec4 next) { return accumulated + next; }
#endif

// If no initializer action is provided, the default behaviour is to initialize to zero.
#ifndef PRE_COMBINE_OP
    #define PRE_COMBINE_OP vec4 pre_combine() { return vec4(0); }
#endif

// If no post-combine function is provided, the default behaviour is to divide by the number of textures,
// so an average is generated.
#ifndef POST_COMBINE_OP
    #define POST_COMBINE_OP vec4 post_combine(vec4 accumulated) { return accumulated / float(num_populated_textures); }
#endif


uniform sampler2D textures[SAMPLER_ARRAY_SIZE];
UBO U_TexArrayData { uint num_populated_textures; };

in NO_VERTEX_BLOCK vertex;
out vec4 color;


COMBINE_TEXTURES_OP
PRE_COMBINE_OP
POST_COMBINE_OP


// Generic shader for combining an array of textures in some way.
// This shader can be compiled with different functions for COMBINE_TEXTURES_OP, PRE_COMBINE_OP
// and POST_COMBINE_OP to control its behaviour.
// The default behaviour is to return the average of all textures.
void main() {
    vec4 result = pre_combine();

    // Loop indices are considered constant-index-expressions, so we don't need to unroll or use sample_array.
    for (uint i = 0; i < num_populated_textures; ++i) {
        result = combine(result, texture(textures[i], vertex.uv));
    }

    color = post_combine(result);
}