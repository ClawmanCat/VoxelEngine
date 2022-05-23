#version 430

#include "utility/math.util.glsl"
#include "utility/edgecase.util.glsl"
#include "structs/bloom.util.glsl"
#include "structs/vertex.util.glsl"


UBO U_GaussianData { GaussianData data; };
UBO U_GaussianDirection { uint direction; };

uniform sampler2D tex;

in NO_VERTEX_BLOCK vertex;
out vec4 color;


// 2-pass gaussian blur shader. Users should pass direction = GAUSSIAN_HORIZONTAL for first pass
// and direction = GAUSSIAN_VERTICAL for the second pass, or vice versa.
// This shader should be used in conjunction with screen_quad.g_input.glsl as its vertex shader.
void main() {
    vec2 delta = vec2(1.0 / textureSize(tex, 0)) * data.range;

    vec4 color_rgba = texture(tex, vertex.uv);
    vec3 color_rgb  = color_rgba.rgb * data.weights[0];

    if (direction == GAUSSIAN_HORIZONTAL) {
        for (uint i = 1; i < data.populated_weights; ++i) {
            color_rgb += texture(tex, vertex.uv + vec2(delta.x * i, 0.0)).rgb * data.weights[i];
            color_rgb += texture(tex, vertex.uv - vec2(delta.x * i, 0.0)).rgb * data.weights[i];
        }
    }

    else {
        for (uint i = 1; i < data.populated_weights; ++i) {
            color_rgb += texture(tex, vertex.uv + vec2(0.0, delta.y * i)).rgb * data.weights[i];
            color_rgb += texture(tex, vertex.uv - vec2(0.0, delta.y * i)).rgb * data.weights[i];
        }
    }

    color = vec4(color_rgb, color_rgba.a);
}
