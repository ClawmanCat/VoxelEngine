#version 430

#include "utility/preprocessor.util.glsl"
#include "structs/camera.util.glsl"
#include "structs/vertex.util.glsl"


UBO U_Camera { Camera camera; };
UBO U_Transform { mat4 transform; };

in vec3 position;
in vec2 uv;
in uint texture_index;

out TEX_VERTEX_BLOCK vertex;


// Basic shader for rendering 3D objects textured from an atlas texture.
// This shader does not perform any lighting calculations.
void main() {
    INIT_TEX_VERTEX_BLOCK(vertex);

    gl_Position = transform * vec4(vertex.position, 1.0f);
    gl_Position = camera.matrix * gl_Position;
}
