#version 430

#include "utility/pbr.util.glsl"
#include "utility/math.util.glsl"
#include "utility/edgecase.util.glsl"
#include "utility/sampler_array.util.glsl"
#include "structs/camera.util.glsl"
#include "structs/light.util.glsl"
#include "structs/bloom.util.glsl"
#include "structs/vertex.util.glsl"


UBO U_Camera { Camera camera; };
UBO U_Lighting { LightingData light_data; };

#ifdef F_ENABLE_BLOOM
    UBO U_BloomData { BloomData bloom_data; };
#endif


uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_color;
// R = Roughness, G = Metalness, B = Ambient Occlusion, A = Emission.
uniform sampler2D g_material;


in NO_VERTEX_BLOCK vertex;

out vec4 l_position;
out vec4 l_color;

#ifdef F_ENABLE_BLOOM
    out vec4 l_bloom;
#endif


void main() {
    l_position = texture(g_position, vertex.uv);
    l_color    = texture(g_color, vertex.uv);

    vec3 normal   = texture(g_normal, vertex.uv).xyz;
    vec4 material = texture(g_material, vertex.uv);

    float roughness = material.r;
    float metalness = material.g;
    float occlusion = material.b;
    float emissive  = material.a;


    // Reflection at an incidence angle of zero.
    vec3 reflection_at_zero = mix(f_dielectric, l_color.rgb, metalness);
    // Direction from which the camera views the fragment.
    vec3 eye_on_fragment = normalize(camera.position - l_position.xyz);
    // Cosine of the angle between the view direction and the surface normal.
    float eye_alignment = max(dot(normal, eye_on_fragment), 0.0);


    vec3 total_radiance = vec3(0.0);
    for (uint i = 0; i < light_data.num_populated_lights; ++i) {
        Light light = light_data.lights[i];

        // Direction from which the fragment is hit by the light.
        vec3 light_on_fragment = normalize(light.position - l_position.xyz);
        // Cosine of the angle between the light direction and the surface normal.
        float light_alignment = max(dot(normal, light_on_fragment), 0.0);
        // Halfway vector used to calculate microfacet alignment.
        vec3 halfway = normalize(eye_on_fragment + light_on_fragment);
        // Distance between the light and the fragment.
        float distance_to_light = length(light.position - l_position.xyz);


        // Attenuation is the loss of light intensity over distance.
        float attenuation = 1.0 / pow(distance_to_light + epsilon, light.attenuation);
        // Observed radiance at the position of the fragment.
        vec3 radiance = light.radiance * attenuation;


        // Calculate Cook-Torrance BRDF
        // i.e. the specular reflectivity of the fragment as a function of its alignment with the eye's view of the fragment.
        float NDF = distribution_GGX(normal, halfway, roughness);
        float G   = geometry_smith(eye_alignment, light_alignment, roughness);
        vec3  F   = fresnel_schlick(max(dot(halfway, eye_on_fragment), 0.0), reflection_at_zero);

        vec3 specular = (NDF * G * F) / unzero(4.0 * eye_alignment * light_alignment);


        // Light is either reflected specularly or diffusely, depending on the calculated Fresnel value.
        vec3 energy_specular = F;
        vec3 energy_diffuse  = (vec3(1.0) - F) * (1.0 - metalness);

        total_radiance += (energy_diffuse * l_color.rgb / pi + specular) * radiance * light_alignment;
    }


    l_color.rgb =
        (light_data.ambient_light * l_color.rgb * occlusion) +
        (emissive * l_color.rgb * light_data.emissivity_constant) +
        total_radiance;


    #ifdef F_ENABLE_BLOOM
        float brightness = dot(l_color.rgb, bloom_data.luma_conversion_weights);
        l_bloom = vec4(l_color.rgb * brightness, 1.0);
    #endif
}
