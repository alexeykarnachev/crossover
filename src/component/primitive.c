#include "../component.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>

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

Primitive init_triangle_primitive(Vec2 b, Vec2 c) {
    Primitive primitive;
    primitive.type = TRIANGLE_PRIMITIVE;
    Triangle triangle = {b, c};
    primitive.p.triangle = triangle;
    return primitive;
}

Primitive init_line_primitive(Vec2 b) {
    Primitive primitive;
    primitive.type = LINE_PRIMITIVE;
    Line line = {b};
    primitive.p.line = line;
    return primitive;
}

static int get_triangle_vertices(Triangle triangle, Vec2* out) {
    Vec2 a = {0.0, 0.0};
    Vec2 b = add(a, triangle.b);
    Vec2 c = add(a, triangle.c);

    out[0] = a;
    out[1] = b;
    out[2] = c;

    Vec2 origin = scale(add_many(out, 3), 1.0 / 3.0);

    out[0] = sub(out[0], origin);
    out[1] = sub(out[1], origin);
    out[2] = sub(out[2], origin);

    return 3;
}

static int get_rectangle_vertices(Rectangle rectangle, Vec2* out) {
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

static int get_line_vertices(Line line, Vec2* out) {
    Vec2 origin = scale(line.b, 0.5);

    Vec2 a = {0.0, 0.0};
    Vec2 b = add(a, line.b);

    out[0] = sub(a, origin);
    out[1] = sub(b, origin);

    return 2;
}

int get_primitive_vertices(Primitive primitive, Vec2* out) {
    PrimitiveType type = primitive.type;
    Vec2 origin;
    int n;

    switch (type) {
        case CIRCLE_PRIMITIVE:
            n = 0;
        case RECTANGLE_PRIMITIVE:
            n = get_rectangle_vertices(primitive.p.rectangle, out);
            break;
        case TRIANGLE_PRIMITIVE:
            n = get_triangle_vertices(primitive.p.triangle, out);
            break;
        case LINE_PRIMITIVE:
            n = get_line_vertices(primitive.p.line, out);
            break;
        default: {
            PRIMITIVE_TYPE_ERROR("get_primitive_vertices", type);
        }
    }

    return n;
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
        Vec2 vertices[4];
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
