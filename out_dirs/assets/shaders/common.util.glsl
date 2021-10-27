// Coeficient for the logarithmic depth buffer.
// (Normally based on the far plane, but since we have infinite perspective, just use a sufficiently large number.)
const float f_coef = 2.0 / log2(1e9 + 1.0);


struct Camera {
    mat4 matrix;
    vec3 position;
    float near;
};


struct Light {
    vec3 position;
    vec3 radiance;
    float attenuation;
};


float scale(float value, vec2 from, vec2 to) {
    return (((value - from.x) * (to.y - to.x)) / (from.y - from.x)) + to.x;
}