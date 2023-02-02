#include "../component.h"
#include "../world.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRIMITIVE_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the primitive with type id: %d. Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

Primitive init_circle_primitive(float radius) {
    Primitive primitive;
    primitive.type = CIRCLE_PRIMITIVE;
    Circle circle = {radius};
    primitive.p.circle = circle;
    return primitive;
}

Primitive init_rectangle_primitive(float width, float height) {
    Primitive primitive;
    primitive.type = RECTANGLE_PRIMITIVE;
    Rectangle rectangle = {width, height};
    primitive.p.rectangle = rectangle;
    return primitive;
}

Primitive init_line_primitive(Vec2 b) {
    Primitive primitive;
    primitive.type = LINE_PRIMITIVE;
    Line line = {b};
    primitive.p.line = line;
    return primitive;
}

Primitive init_polygon_primitive(
    Vec2 vertices[MAX_N_POLYGON_VERTICES], int n_vertices
) {
    Primitive primitive;
    primitive.type = POLYGON_PRIMITIVE;
    Polygon polygon;
    memcpy(polygon.vertices, vertices, sizeof(Vec2) * n_vertices);
    polygon.n_vertices = n_vertices;
    primitive.p.polygon = polygon;
    return primitive;
}

static int get_rectangle_vertices(
    Rectangle rectangle, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    Vec2 origin = vec2(0.5 * rectangle.width, 0.5 * rectangle.height);

    Vec2 d = {0.0, 0.0};
    Vec2 a = {d.x, d.y + rectangle.height};
    Vec2 b = {a.x + rectangle.width, a.y};
    Vec2 c = {b.x, d.y};

    out[0] = sub(a, origin);
    out[1] = sub(b, origin);
    out[2] = sub(c, origin);
    out[3] = sub(d, origin);

    return 4;
}

static int get_line_vertices(Line line, Vec2 out[MAX_N_POLYGON_VERTICES]) {
    Vec2 origin = scale(line.b, 0.5);

    Vec2 a = {0.0, 0.0};
    Vec2 b = add(a, line.b);

    out[0] = sub(a, origin);
    out[1] = sub(b, origin);

    return 2;
}

static int get_polygon_vertices(
    Polygon polygon, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    memcpy(out, polygon.vertices, sizeof(Vec2) * polygon.n_vertices);
    return polygon.n_vertices;
}

static int get_line_fan_vertices(
    Line line, float width, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    Vec2 normal = normalize(rotate90(line.b));
    Vec2 half_width = scale(normal, width * 0.5);
    get_line_vertices(line, out);

    Vec2 a = out[0];
    Vec2 b = out[1];

    out[0] = add(a, half_width);
    out[1] = sub(a, half_width);
    out[2] = sub(b, half_width);
    out[3] = add(b, half_width);

    return 4;
}

int get_primitive_vertices(
    Primitive primitive, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    PrimitiveType type = primitive.type;
    int n_vertices;

    switch (type) {
        case CIRCLE_PRIMITIVE:
            n_vertices = 0;
        case RECTANGLE_PRIMITIVE:
            n_vertices = get_rectangle_vertices(
                primitive.p.rectangle, out
            );
            break;
        case LINE_PRIMITIVE:
            n_vertices = get_line_vertices(primitive.p.line, out);
            break;
        case POLYGON_PRIMITIVE:
            n_vertices = get_polygon_vertices(primitive.p.polygon, out);
            break;
        default: {
            PRIMITIVE_TYPE_ERROR("get_primitive_vertices", type);
        }
    }

    return n_vertices;
}

int get_primitive_fan_vertices(
    Primitive primitive, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    PrimitiveType type = primitive.type;
    int n_vertices;

    switch (type) {
        case LINE_PRIMITIVE: {
            float line_width = WORLD.camera_view_width * LINE_WIDTH_SCALE;
            n_vertices = get_line_fan_vertices(
                primitive.p.line, line_width, out
            );
            break;
        }
        default:
            n_vertices = get_primitive_vertices(primitive, out);
    }

    return n_vertices;
}

Rectangle get_primitive_bounding_rectangle(
    Primitive primitive, Transformation transformation
) {
    Vec2 position = transformation.position;

    float width = 0;
    float height = 0;
    if (primitive.type == CIRCLE_PRIMITIVE) {
        width = 2.0 * primitive.p.circle.radius;
        height = width;
    } else {
        Vec2 vertices[MAX_N_POLYGON_VERTICES];
        int n_vertices = get_primitive_vertices(primitive, vertices);
        apply_transformation(vertices, n_vertices, transformation);

        for (int i = 0; i < n_vertices; ++i) {
            Vec2 vertex = vertices[i];
            width = max(width, 2.0 * fabs(position.x - vertex.x));
            height = max(height, 2.0 * fabs(position.y - vertex.y));
        }
    }

    Rectangle rectangle = {width, height};
    return rectangle;
}
