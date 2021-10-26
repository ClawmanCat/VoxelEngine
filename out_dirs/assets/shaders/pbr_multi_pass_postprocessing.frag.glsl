#version 430
#include "common.util.glsl"
#include "pbr.util.glsl"


uniform sampler2D l_position;
uniform sampler2D l_color;

in vec2 uv;

out vec4 color;


void main() {
    color = linear_to_SRGB(texture(l_color, uv));
    gl_FragDepth = texture(l_position, uv).w;
}