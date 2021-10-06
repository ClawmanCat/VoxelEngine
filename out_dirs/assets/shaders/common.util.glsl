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