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

in vec2 vs_world_pos;
in vec2 vs_uv_pos;

uniform Circle circle;
uniform Camera camera;
uniform int primitive_type;
uniform float render_layer;

out vec2 fs_uv_pos;

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
void get_circle_attributes(out vec2 world_pos, out vec2 uv_pos) {
    int id = gl_VertexID;

    world_pos = circle.position;
    if (id > 0) {
        world_pos.x += circle.radius;
        float angle = (id - 1) * 2.0 * PI / float(circle.n_polygons);
        uv_pos = 0.5 * (vec2(cos(angle), sin(angle)) + 1.0);
        world_pos = rotate(world_pos, circle.position, angle);
    } else {
        uv_pos = vec2(0.5);
    }
}

void main(void) {
    vec2 world_position;
    if (primitive_type == 0) {
        get_circle_attributes(world_position, fs_uv_pos);
    } else {
        world_position = vs_world_pos;
        fs_uv_pos = vs_uv_pos;
    }

    vec2 proj_pos = world2proj(world_position); 
    gl_Position = vec4(proj_pos, render_layer, 1.0);
}
