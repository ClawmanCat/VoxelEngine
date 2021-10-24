#version 430
#include "common.util.glsl"
#include "pbr.util.glsl"


const float pi = 3.14159265;

// Sufficiently small number to prevent division by zero.
const float epsilon = 1e-4;
// Normal incidence Fresnel factor for dielectrics.
const vec3 f_dielectric = vec3(0.04);


layout (std140) uniform U_Camera {
    Camera camera;
};

layout (std140) uniform U_Transform {
    mat4 transform;
};

layout (std140) uniform U_Lighting {
    Light light;
    vec3 ambient_light;
};


uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_color;
uniform sampler2D g_material;


out vec4 l_position;
out vec4 l_color;


void main() {
    l_position = texture(g_position, gl_FragCoord.xy);
    l_color    = texture(g_color, gl_FragCoord.xy);


    vec4 diffuse = texture(g_color, gl_FragCoord.xy);
    if (diffuse.a == 0.0) discard; // Don't waste time shading transparent pixels.


    vec3 normal  = texture(g_normal, gl_FragCoord.xy).xyz;

    vec4 material = texture(g_material, gl_FragCoord.xy);
    float roughness = material.r;
    float metalness = material.g;
    float occlusion = material.b;


    // Reflection at an incidence angle of zero.
    vec3 reflection_at_zero = mix(f_dielectric, color.rgb, metalness);
    // Direction from which the camera views the fragment.
    vec3 eye_on_fragment = normalize(camera.position - position.xyz);
    // Cosine of the angle between the view direction and the surface normal.
    float eye_alignment = max(dot(normal, eye_on_fragment), 0.0);


    // Direction from which the fragment is hit by the light.
    vec3 light_on_fragment = normalize(light.position - position.xyz);
    // Cosine of the angle between the light direction and the surface normal.
    float light_alignment = max(dot(normal, light_on_fragment), 0.0);
    // Halfway vector used to calculate microfacet alignment.
    vec3 halfway = normalize(eye_on_fragment + light_on_fragment);
    // Distance between the light and the fragment.
    float distance_to_light = length(light.position - position.xyz);

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


    l_color.rgb += (energy_diffuse * diffuse.rgb / pi + specular) * radiance * light_alignment;
}