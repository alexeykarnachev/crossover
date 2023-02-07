#version 460 core

#define PI 3.14159265359

struct Camera {
    vec2 position;
    float orientation;
    float view_width;
    float view_height;
};

struct Circle {
    vec2 position;
    float radius;
    int n_polygons;
};

in vec2 world_pos;

uniform Circle circle;
uniform Camera camera;
uniform int type;
uniform float render_layer;

vec2 rotate(vec2 point, vec2 center, float angle) {
    vec2 p0 = point - center;
    float c = cos(angle);
    float s = sin(angle);
    vec2 p1 = vec2(p0.x * c - p0.y * s, p0.y * c + p0.x * s);
    p1 += center;
    return p1;
}

vec2 world2proj(vec2 world_pos) {
    vec2 half_size = vec2(0.5 * vec2(camera.view_width, camera.view_height));

    vec2 view_pos = rotate(world_pos, vec2(0.0, 0.0), -camera.orientation);
    view_pos -= camera.position;

    return view_pos / half_size;
}

// Render with TRIANGLE_FAN
vec2 get_circle_position() {
    int id = gl_VertexID;

    vec2 world_pos = circle.position;
    if (id > 0) {
        world_pos.x += circle.radius;
        float angle = (id - 1) * 2.0 * PI / float(circle.n_polygons);
        world_pos = rotate(world_pos, circle.position, angle);
    }

    return world_pos;
}

void main(void) {
    vec2 world_position;
    if (type == 0) {
        world_position = get_circle_position();
    } else {
        world_position = world_pos;
    }

    vec2 proj_pos = world2proj(world_position); 
    gl_Position = vec4(proj_pos, render_layer, 1.0);
}
