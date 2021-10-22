const float pi = 3.14159265;
// Sufficiently small number to prevent division by zero.
const float epsilon = 1e-4;
// Normal incidence Fresnel factor for dielectrics.
const vec3 f_dielectric = vec3(0.04);


// Trowbridge-Reitz GGX normal distribution.
float distribution_GGX(vec3 normal, vec3 halfway, float roughness) {
    float roughness_quad = pow(roughness, 4.0);
    // Cosine of the angle between the halfway vector and the surface normal.
    float halfway_alignment = max(dot(normal, halfway), 0.0);

    float denominator = ((halfway_alignment * halfway_alignment) * (roughness_quad - 1.0) + 1.0);
    denominator = pi * denominator * denominator;

    return roughness_quad / denominator;
}


// Combined Schlick-Beckmann GGX geometry function.
float geometry_schlick_GGX(float view_normal_alignment, float roughness) {
    float k = 1.0 + roughness;
    k = (k * k) / 8.0;

    return view_normal_alignment / (view_normal_alignment * (1.0 - k) + k);
}


// Use Smith's method to combine influences on the perceived geometry from both the light and eye alignments.
float geometry_smith(vec3 normal, float eye_alignment, float light_alignment, float roughness) {
    return geometry_schlick_GGX(eye_alignment, roughness) *
    geometry_schlick_GGX(light_alignment, roughness);
}


// Fresnel-Schlick approximation for the Fresnel equation.
vec3 fresnel_schlick(float light_alignment, vec3 reflection_at_zero) {
    return reflection_at_zero + (1.0 - reflection_at_zero) * pow(clamp(1.0 - light_alignment, 0.0, 1.0), 5.0);
}