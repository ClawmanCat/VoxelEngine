#version 430
#include "common.util.glsl"


#ifndef GAUSSIAN_SIZE_LIMIT
    #define GAUSSIAN_SIZE_LIMIT 16
#endif

#define GAUSSIAN_HORIZONTAL 0
#define GAUSSIAN_VERTICAL   1


layout (std140, binding = 0) uniform U_GaussianData {
    float weights[GAUSSIAN_SIZE_LIMIT];
    uint populated_weights;
    float range;
};

layout (std140, binding = 1) uniform U_GaussianDirection {
    uint direction;
};

uniform sampler2D tex;
uniform sampler2D position;


in vec2 uv;
out vec4 color;


// 2-pass gaussian blur shader. Users should pass direction = GAUSSIAN_HORIZONTAL for first pass
// and direction = GAUSSIAN_VERTICAL for the second pass, or vice versa.
// This shader should be used in conjunction with screen_quad.g_input.glsl as its vertex shader.
void main() {
    float depth = texture(position, uv).w;

    // Divide by 1 - distance, otherwise the distance between sample points would get larger and larger with distance.
    vec2 delta = (1.0 / textureSize(tex, 0)) * (range * (1 - depth));
    vec4 color_rgba = texture(tex, uv);
    vec3 color_rgb  = color_rgba.rgb * weights[0];

    if (direction == GAUSSIAN_HORIZONTAL) {
        for (uint i = 1; i < populated_weights; ++i) {
            color_rgb += texture(tex, uv + vec2(delta.x * i, 0.0)).rgb * weights[i];
            color_rgb += texture(tex, uv - vec2(delta.x * i, 0.0)).rgb * weights[i];
        }
    }

    else {
        for (uint i = 1; i < populated_weights; ++i) {
            color_rgb += texture(tex, uv + vec2(0.0, delta.y * i)).rgb * weights[i];
            color_rgb += texture(tex, uv - vec2(0.0, delta.y * i)).rgb * weights[i];
        }
    }

    color = vec4(color_rgb, color_rgba.a);
}
