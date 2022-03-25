#pragma once


// Note: engine uses infinite perspective matrix, so there is no far plane.
struct Camera {
    mat4 matrix;
    vec3 position;
    float near;
};
