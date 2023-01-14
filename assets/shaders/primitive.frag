#version 460 core

#define PI 3.14159265359

uniform vec3 diffuse_color;

out vec4 frag_color;

void main(void) {
    frag_color = vec4(diffuse_color, 1.0);
}
