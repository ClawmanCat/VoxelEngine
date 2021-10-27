#version 430
#include "common.util.glsl"
#include "pbr.util.glsl"


layout (std140, binding = 0) uniform U_Camera {
    Camera camera;
};

layout (std140, binding = 1) uniform U_Transform {
    mat4 transform;
};

// Should be large enough for the number of lights used.
// TODO: Use shader preprocessor to dynamically recompile with actual value.
const int num_lights = 128;

layout (std140, binding = 2) uniform U_Lighting {
    Light lights[num_lights];
    uint num_populated_lights;

    vec3 ambient_light;
};


const uint num_samplers = VE_MAX_FS_SAMPLERS;
uniform sampler2D textures[num_samplers];



in flat uint frag_tex_index;
in vec2 frag_uv_color;
in vec2 frag_uv_normal;
// R = roughness, G = metalness, B = ambient occlusion.
in vec2 frag_uv_material;

in vec3 frag_position;
in float frag_log_z;

in flat mat3 TBN;


out vec4 color;


void main() {
    vec4 diffuse = texture(textures[frag_tex_index], frag_uv_color);
    if (diffuse.a == 0.0) discard; // Don't waste time shading transparent pixels.

    // Assume the diffuse texture to be in sRGB, and convert to linear before use.
    diffuse.rgb = pow(diffuse.rgb, vec3(2.2));


    vec4 material = texture(textures[frag_tex_index], frag_uv_material);
    float roughness = material.r;
    float metalness = material.g;
    float occlusion = material.b;


    // Reflection at an incidence angle of zero.
    vec3 reflection_at_zero = mix(f_dielectric, diffuse.rgb, metalness);


    vec3 normal = texture(textures[frag_tex_index], frag_uv_normal).xyz;
    normal = 2.0 * (normal - 0.5);
    normal = normalize(TBN * normal);


    // Direction from which the camera views the fragment.
    vec3 eye_on_fragment = normalize(camera.position - frag_position);
    // Cosine of the angle between the view direction and the surface normal.
    float eye_alignment = max(dot(normal, eye_on_fragment), 0.0);


    vec3 total_radiance = vec3(0.0);
    for (uint i = 0; i < num_populated_lights; ++i) {
        // Direction from which the fragment is hit by the light.
        vec3 light_on_fragment = normalize(lights[i].position - frag_position);
        // Cosine of the angle between the light direction and the surface normal.
        float light_alignment = max(dot(normal, light_on_fragment), 0.0);
        // Halfway vector used to calculate microfacet alignment.
        vec3 halfway = normalize(eye_on_fragment + light_on_fragment);
        // Distance between the light and the fragment.
        float distance_to_light = length(lights[i].position - frag_position);

        // Attenuation is the loss of light intensity over distance.
        float attenuation = 1.0 / pow(distance_to_light, lights[i].attenuation);
        // Observed radiance at the position of the fragment.
        vec3 radiance = lights[i].radiance * attenuation;


        // Calculate Cook-Torrance BRDF
        // i.e. the specular reflectivity of the fragment as a function of its alignment with the eye's view of the fragment.
        float NDF = distribution_GGX(normal, halfway, roughness);
        float G   = geometry_smith(normal, eye_alignment, light_alignment, roughness);
        vec3  F   = fresnel_schlick(max(dot(halfway, eye_on_fragment), 0.0), reflection_at_zero);

        vec3 specular = (NDF * G * F) / (4.0 * eye_alignment * light_alignment + epsilon);


        // Light is either reflected specularly or diffusely, depending on the calculated Fresnel value.
        vec3 energy_specular = F;
        vec3 energy_diffuse  = (vec3(1.0) - F) * (1.0 - metalness);


        total_radiance += (energy_diffuse * diffuse.rgb / pi + specular) * radiance * light_alignment;
    }


    // Provide some base ambient lighting to prevent unlit textures from being completely black.
    vec3 color_rgb = (ambient_light * diffuse.rgb * occlusion) + total_radiance;
    // Convert from HDR / linear back to sRGB.
    color_rgb /= (color_rgb + vec3(1.0));
    color_rgb  = pow(color_rgb, vec3(1.0 / 2.2));

    color = vec4(color_rgb, diffuse.a);


    // Store log2(depth) to better handle large distances.
    gl_FragDepth = log2(frag_log_z) * (0.5 * f_coef);
}