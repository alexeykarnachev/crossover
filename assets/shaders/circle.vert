#version 460 core

#define PI 3.14159265359

struct Camera {
    vec2 position;
    float orientation;
    float view_width;
    float view_height;
};

in vec2 vs_pos;
in vec4 vs_geometry;  // vec3(x, y, elevation, radius)
in float vs_render_layer;

uniform Camera camera;

out vec3 fs_world_pos;
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

void main(void) {
    float radius = vs_geometry.w;
    vec2 translation = vs_geometry.xy;
    vec2 world_pos = vs_pos * radius + translation;
    vec2 proj_pos = world2proj(world_pos);
    gl_Position = vec4(proj_pos, vs_render_layer, 1.0);
}
