#pragma once


const float pi = 3.14159265;
// Sufficiently small number to prevent division by zero.
const float epsilon = 1e-6;

float positive_infinity = uintBitsToFloat(0x7F800000);
float negative_infinity = uintBitsToFloat(0xFF800000);


float scale(float value, vec2 from, vec2 to) {
    return (((value - from.x) * (to.y - to.x)) / (from.y - from.x)) + to.x;
}


float distance2(vec2 x, vec2 y) {
    vec2 delta = y - x;
    return dot(delta, delta);
}


float distance2(vec3 x, vec3 y) {
    vec3 delta = y - x;
    return dot(delta, delta);
}


float length2(vec2 x) {
    return distance2(vec2(0), x);
}


float length2(vec3 x) {
    return distance2(vec3(0), x);
}


mat4 look_at(vec3 eye, vec3 look_at) {
    vec3 az = normalize(look_at - eye);
    vec3 ay = normalize(cross(vec3(0, 1, 0), az));
    vec3 ax = cross(az, ay);

    return mat4(
    ax.x,          ay.x,          az.x,          0,
    ax.y,          ay.y,          az.y,          0,
    ax.z,          ay.z,          az.z,          0,
    dot(ax, -eye), dot(ay, -eye), dot(az, -eye), 1
    );
}


// Generates a TBN matrix, that is, a matrix to transform normals from vertex space to world space.
mat3 TBN_matrix(mat4 transform, vec3 normal, vec3 tangent) {
    vec3 bitangent = normalize(cross(normal, tangent));

    vec3 T = normalize((transform * vec4(tangent,   0.0)).xyz);
    vec3 B = normalize((transform * vec4(bitangent, 0.0)).xyz);
    vec3 N = normalize((transform * vec4(normal,    0.0)).xyz);

    return mat3(T, B, N);
}
