#version 460 core

#define PI 3.14159265359

struct Circle {
    int n_polygons;
    float radius;
    vec2 position;
    vec3 color;
};

struct Camera {
    float aspect_ratio;
    float elevation;
    vec2 position;
};

uniform Circle circle;
uniform Camera camera;

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

    vec2 world_pos = circle.position;
    if (id > 0) {
        world_pos.x += circle.radius;
        float angle = (id - 1) * 2.0 * PI / float(circle.n_polygons);
        world_pos = rotate(world_pos, circle.position, angle);
    }

    vec2 view_pos = world_pos - camera.position;
    float view_width = 2.0 * camera.elevation * tan(0.25 * PI);
    float view_height = view_width / camera.aspect_ratio;
    vec2 proj_pos = (view_pos - camera.position) / (0.5 * vec2(view_width, view_height));

    gl_Position = vec4(proj_pos, 0.0, 1.0);
}
