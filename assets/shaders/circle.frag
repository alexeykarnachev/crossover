#version 460 core

#define PI 3.14159265359

struct Circle {
    int n_polygons;
    float radius;
    vec2 position;
    vec3 color;
};

uniform Circle circle;

out vec4 frag_color;

void main(void) {
    frag_color = vec4(circle.color, 1.0);
}
