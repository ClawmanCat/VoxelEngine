#version 430

uniform sampler2D l_position;
uniform sampler2D l_color;

out vec4 color;


// TODO: Apply bloom here. g_color is already in HDR color space, so we just need to sample different mip levels.
void main() {
    color = texture(l_color, gl_FragCoord.xy);

    // Convert from HDR / linear back to sRGB.
    color.rgb /= (color.rgb + vec3(1.0));
    color.rgb  = pow(color.rgb, vec3(1.0 / 2.2));


    gl_FragColor = color;
    gl_FragDepth = texture(l_position, gl_FragCoord.xy).w;
}
