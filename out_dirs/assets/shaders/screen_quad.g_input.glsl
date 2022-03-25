#version 430

#include "utility/multipass.util.glsl"
#include "structs/vertex.util.glsl"


out NO_VERTEX_BLOCK vertex;


void main() {
    // Generate a screen-filling quad to render to.
    gl_Position = vec4(quad_vertices[gl_VertexID], 0.0, 1.0);
    vertex.uv = (quad_vertices[gl_VertexID] + 1.0) / 2.0;
}
