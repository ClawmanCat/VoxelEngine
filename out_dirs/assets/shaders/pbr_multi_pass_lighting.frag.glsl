#version 430
#include "common.util.glsl"
#include "pbr.util.glsl"


layout (std140, binding = 0) uniform U_Camera {
    Camera camera;
};

layout (std140, binding = 3) uniform U_PerLightData {
    Light light;
};


uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_color;
uniform sampler2D g_color_initial;
uniform sampler2D g_material;

in vec2 uv;

out vec4 l_position;
out vec4 l_normal;
out vec4 l_color;
out vec4 l_material;


void main() {
    l_position = texture(g_position, uv);
    l_color    = texture(g_color, uv);
    l_normal   = texture(g_normal, uv);
    l_material = texture(g_material, uv);

    float roughness = l_material.r;
    float metalness = l_material.g;
    float occlusion = l_material.b;

    vec3 src_color = texture(g_color_initial, uv).rgb;
    vec3 normal = l_normal.xyz;


    // Reflection at an incidence angle of zero.
    vec3 reflection_at_zero = mix(f_dielectric, l_color.rgb, metalness);
    // Direction from which the camera views the fragment.
    vec3 eye_on_fragment = normalize(camera.position - l_position.xyz);
    // Cosine of the angle between the view direction and the surface normal.
    float eye_alignment = max(dot(normal, eye_on_fragment), 0.0);


    // Direction from which the fragment is hit by the light.
    vec3 light_on_fragment = normalize(light.position - l_position.xyz);
    // Cosine of the angle between the light direction and the surface normal.
    float light_alignment = max(dot(normal, light_on_fragment), 0.0);
    // Halfway vector used to calculate microfacet alignment.
    vec3 halfway = normalize(eye_on_fragment + light_on_fragment);
    // Distance between the light and the fragment.
    float distance_to_light = length(light.position - l_position.xyz);


    // Attenuation is the loss of light intensity over distance.
    float attenuation = 1.0 / pow(distance_to_light, light.attenuation);
    // Observed radiance at the position of the fragment.
    vec3 radiance = light.radiance * attenuation;


    // Calculate Cook-Torrance BRDF
    // i.e. the specular reflectivity of the fragment as a function of its alignment with the eye's view of the fragment.
    float NDF = distribution_GGX(normal, halfway, roughness);
    float G   = geometry_smith(normal, eye_alignment, light_alignment, roughness);
    vec3  F   = fresnel_schlick(max(dot(halfway, eye_on_fragment), 0.0), reflection_at_zero);

    vec3 specular = (NDF * G * F) / (4.0 * eye_alignment * light_alignment + epsilon);


    // Light is either reflected specularly or diffusely, depending on the calculated Fresnel value.
    vec3 energy_specular = F;
    vec3 energy_diffuse  = (vec3(1.0) - F) * (1.0 - metalness);

    l_color.rgb += (energy_diffuse * src_color / pi + specular) * radiance * light_alignment;
}