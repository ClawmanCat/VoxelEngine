#version 430
#include "multipass.util.glsl"


void main() {
    // Generate a screen-filling quad to render to.
    gl_Position = vec4(quad_vertices[gl_VertexID], 0.0, 1.0);
}