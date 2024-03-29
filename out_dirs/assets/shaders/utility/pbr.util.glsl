#pragma once

#include "math.util.glsl"
#include "edgecase.util.glsl"


// Normal incidence Fresnel factor for dielectrics.
const vec3 f_dielectric = vec3(0.04);


// Trowbridge-Reitz GGX normal distribution.
float distribution_GGX(vec3 normal, vec3 halfway, float roughness) {
    float roughness_quad = pow(roughness, 4.0);
    // Cosine of the angle between the halfway vector and the surface normal.
    float halfway_alignment = max(dot(normal, halfway), 0.0);

    float denominator = ((halfway_alignment * halfway_alignment) * (roughness_quad - 1.0) + 1.0);
    denominator = pi * denominator * denominator;

    return roughness_quad / unzero(denominator);
}


// Combined Schlick-Beckmann GGX geometry function.
float geometry_schlick_GGX(float view_normal_alignment, float roughness) {
    float k = 1.0 + roughness;
    k = (k * k) / 8.0;

    return view_normal_alignment / unzero(view_normal_alignment * (1.0 - k) + k);
}


// Use Smith's method to combine influences on the perceived geometry from both the light and eye alignments.
float geometry_smith(float eye_alignment, float light_alignment, float roughness) {
    return (
    geometry_schlick_GGX(eye_alignment,   roughness) *
    geometry_schlick_GGX(light_alignment, roughness)
    );
}


// Fresnel-Schlick approximation for the Fresnel equation.
vec3 fresnel_schlick(float light_alignment, vec3 reflection_at_zero) {
    return reflection_at_zero + (1.0 - reflection_at_zero) * pow(clamp(1.0 - light_alignment, 0.0, 1.0), 5.0);
}


vec4 SRGB_to_linear(vec4 color) {
    return vec4(pow(color.rgb, vec3(2.2)), color.a);
}


vec4 linear_to_SRGB(vec4 color) {
    color.rgb /= (color.rgb + vec3(1.0));
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));

    return color;
}


vec4 tone_map(vec4 color, float exposure) {
    return vec4(vec3(1.0) - exp(-color.rgb * exposure), color.a);
}
