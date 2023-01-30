#version 460 core

#define PI 3.14159265359

struct Camera {
    vec2 position;
    float orientation;
    float view_width;
    float view_height;
};

struct Polygon {
    vec2 a;
    vec2 b;
    vec2 c;
    vec2 d;
};

struct Circle {
    vec2 position;
    float radius;
    int n_polygons;
};

uniform Polygon polygon;
uniform Circle circle;
uniform Camera camera;
uniform int type;

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

// Render with TRIANGLE_STRIP with 4 vertices
vec2 get_rectangle_position() {
    int id = gl_VertexID;
    if (id == 0) {
        return polygon.d;
    } else if (id == 1) {
        return polygon.a;
    } else if (id == 2) {
        return polygon.c;
    } else if (id == 3) {
        return polygon.b;
    }
}

// Render with TRIANGLE_STRIP with 3 vertices
vec2 get_triangle_position() {
    int id = gl_VertexID;
    if (id == 0) {
        return polygon.a;
    } else if (id == 1) {
        return polygon.b;
    } else if (id == 2) {
        return polygon.c;
    }
}

// Render with LINE with 2 vertices
vec2 get_line_position() {
    int id = gl_VertexID;
    if (id == 0) {
        return polygon.a;
    } else if (id == 1) {
        return polygon.b;
    }
}

void main(void) {
    vec2 world_position;
    if ((type & 1 << 0) != 0) {
        world_position = get_circle_position();
    } else if ((type & 1 << 1) != 0) {
        world_position = get_rectangle_position();
    } else if ((type & 1 << 2) != 0) {
        world_position = get_triangle_position();
    } else if ((type & 1 << 3) != 0) {
        world_position = get_line_position();
    }

    vec2 proj_pos = world2proj(world_position); 
    gl_Position = vec4(proj_pos, 0.0, 1.0);
}
