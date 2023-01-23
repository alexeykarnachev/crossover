#include "primitive.h"

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

Circle circle(float radius) {
    Circle circle = {radius};
    return circle;
}

Rectangle rectangle(float width, float height) {
    Rectangle rectangle = {width, height};
    return rectangle;
}

Triangle triangle(Vec2 b, Vec2 c) {
    Triangle triangle = {b, c};
    return triangle;
}

Line line(Vec2 b) {
    Line line = {b};
    return line;
}

Primitive circle_primitive(Circle c) {
    Primitive primitive;
    primitive.type = CIRCLE_PRIMITIVE;
    primitive.p.circle = c;
    return primitive;
}

Primitive rectangle_primitive(Rectangle r) {
    Primitive primitive;
    primitive.type = RECTANGLE_PRIMITIVE;
    primitive.p.rectangle = r;
    return primitive;
}

Primitive triangle_primitive(Triangle t) {
    Primitive primitive;
    primitive.type = TRIANGLE_PRIMITIVE;
    primitive.p.triangle = t;
    return primitive;
}

Primitive line_primitive(Line v) {
    Primitive primitive;
    primitive.type = LINE_PRIMITIVE;
    primitive.p.line = v;
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

int get_primitive_vertices(
    Primitive primitive, Transformation transformation, Vec2* out
) {
    PrimitiveType type = primitive.type;
    PrimitiveOriginType origin_type = BARYCENTRIC_ORIGIN;
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
            printf(
                "%f,%f\n",
                transformation.position.x,
                transformation.position.y
            );

            PRIMITIVE_TYPE_ERROR("get_primitive_vertices", type);
        }
    }

    for (int i = 0; i < n; ++i) {
        out[i] = transform(out[i], transformation);
    }

    return n;
}
