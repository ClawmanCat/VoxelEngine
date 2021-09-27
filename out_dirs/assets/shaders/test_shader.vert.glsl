// Minimal test shader used for system testing.
#version 430


layout (location = 0) in vec3 position;


void main() {
    gl_Position = vec4(position, 1.0f);
}