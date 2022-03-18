const float pi = 3.14159265;
// Sufficiently small number to prevent division by zero.
const float epsilon = 1e-4;


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


// Returns f is f is nonzero, or epsilon otherwise. Used to prevent division by zero.
float unzero(float f) {
    return float(f == 0) * epsilon + f;
}


bool has_nan(vec2 pixel) { return any(isnan(pixel)); }
bool has_nan(vec3 pixel) { return any(isnan(pixel)); }
bool has_nan(vec4 pixel) { return any(isnan(pixel)); }