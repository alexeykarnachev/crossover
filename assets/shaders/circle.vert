#version 460 core

#define PI 3.14159265359

uniform vec2 center;
uniform float radius;
uniform int n_polygons;


vec2 rotate(vec2 point, vec2 center, float angle) {
    vec2 p0 = point - center;
    float c = cos(angle);
    float s = sin(angle);
    vec2 p1 = vec2(p0.x * c - p0.y * s, p0.y * c + p0.x * s);
    p1 += center;
    return p1;
}


void main(void) {
    int id = gl_VertexID;

    vec2 pos = center;
    if (id > 0) {
        vec2 point = center;
        point.x += radius;
        float angle = (id - 1) * PI / n_polygons;
        pos = rotate(point, center, angle);
    }
    gl_Position = vec4(pos, 0.0, 1.0);
}
