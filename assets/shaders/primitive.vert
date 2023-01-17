#version 460 core

#define PI 3.14159265359

struct Camera {
    vec2 position;
    float aspect_ratio;
    float elevation;
};

struct Circle {
    vec2 position;
    float radius;
    int n_polygons;
};

struct Rectangle {
    vec2 position;
    float width;
    float height;
};

struct Triangle {
    vec2 position;
    vec2 b;
    vec2 c;
};

struct Line {
    vec2 position;
    vec2 b;
};

uniform Camera camera;
uniform Circle circle;
uniform Rectangle rectangle;
uniform Triangle triangle;
uniform Line line;
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
    vec2 view_pos = world_pos - camera.position;
    float view_width = 2.0 * camera.elevation * tan(0.25 * PI);
    float view_height = view_width / camera.aspect_ratio;
    vec2 proj_pos = (view_pos - camera.position) / (0.5 * vec2(view_width, view_height));
    return proj_pos;
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

    vec2 world_pos = rectangle.position;
    if (id == 1) {
        world_pos.y += rectangle.height;
    } else if (id == 2) {
        world_pos.x += rectangle.width;
    } else if (id == 3) {
        world_pos.x += rectangle.width;
        world_pos.y += rectangle.height;
    }

    return world_pos;
}

// Render with TRIANGLE_STRIP with 3 vertices
vec2 get_triangle_position() {
    int id = gl_VertexID;

    vec2 world_pos;
    if (id == 0) {
        world_pos = triangle.position;
    } else if (id == 1) {
        world_pos = triangle.position + triangle.b;
    } else if (id == 2) {
        world_pos = triangle.position + triangle.c;
    }

    return world_pos;
}

// Render with LINE with 2 vertices
vec2 get_line_position() {
    int id = gl_VertexID;

    vec2 world_pos;
    if (id == 0) {
        world_pos = line.position;
    } else if (id == 1) {
        world_pos = line.position + line.b;
    }

    return world_pos;
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
