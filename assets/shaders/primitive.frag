#version 460 core

#define PI 3.14159265359

uniform vec4 color;

out vec4 frag_color;

void main(void) {
    frag_color = color;
}
