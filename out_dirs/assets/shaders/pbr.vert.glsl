#version 430

#include "utility/math.util.glsl"
#include "structs/camera.util.glsl"
#include "structs/vertex.util.glsl"


UBO U_Camera { Camera camera; };
UBO U_Transform { mat4 transform; };

in vec3 position;
in vec3 normal;
in vec3 tangent;

in uint texture_index;
in vec2 uv_color;
in vec2 uv_normal;
in vec2 uv_material;

out PBR_VERTEX_BLOCK vertex;
out flat mat3 TBN;


void main() {
    INIT_PBR_VERTEX_BLOCK(vertex);

    vertex.position = (transform * vec4(vertex.position, 1.0)).xyz;
    gl_Position = camera.matrix * vec4(vertex.position, 1.0);

    TBN = TBN_matrix(transform, normal, tangent);
}
