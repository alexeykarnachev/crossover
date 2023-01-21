#include "primitive.h"

#include "math.h"
#include "movement.h"
#include <stdio.h>
#include <stdlib.h>

#define ORIGIN_TYPE_ERROR(primitive, origin_type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't get the origin of the %s primitive with the " \
            "origin type id: %d. Needs to be implemented", \
            primitive, \
            origin_type \
        ); \
        exit(1); \
    } while (0)

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

Circle circle(Vec2 position, float radius, float rotation) {
    Circle circle = {position, radius, rotation};
    return circle;
}

Rectangle rectangle(
    Vec2 position, float width, float height, float rotation
) {
    Rectangle rectangle = {position, width, height, rotation};
    return rectangle;
}

Triangle triangle(Vec2 position, Vec2 b, Vec2 c, float rotation) {
    Triangle triangle = {position, b, c, rotation};
    return triangle;
}

Line line(Vec2 position, Vec2 b, float rotation) {
    Line line = {position, b, rotation};
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
    Vec2 a = triangle.position;
    Vec2 b = add(a, triangle.b);
    Vec2 c = add(a, triangle.c);

    out[0] = a;
    out[1] = b;
    out[2] = c;

    return 3;
}

static int get_rectangle_vertices(Rectangle rectangle, Vec2* out) {
    Vec2 d = rectangle.position;
    Vec2 a = {d.x, d.y + rectangle.height};
    Vec2 b = {a.x + rectangle.width, a.y};
    Vec2 c = {b.x, d.y};

    out[0] = a;
    out[1] = b;
    out[2] = c;
    out[3] = d;

    return 4;
}

static int get_line_vertices(Line line, Vec2* out) {
    Vec2 a = line.position;
    Vec2 b = add(a, line.b);

    out[0] = a;
    out[1] = b;

    return 2;
}

static Vec2 get_circle_origin(
    Circle circle, PrimitiveOriginType origin_type
) {
    switch (origin_type) {
        case BARYCENTRIC_ORIGIN:
            return circle.position;
        default:
            ORIGIN_TYPE_ERROR("Circle", origin_type);
    }
}

static Vec2 get_rectangle_origin(
    Rectangle rectangle, PrimitiveOriginType origin_type
) {
    switch (origin_type) {
        case BARYCENTRIC_ORIGIN: {
            float x = rectangle.position.x + 0.5 * rectangle.width;
            float y = rectangle.position.y + 0.5 * rectangle.height;
            return vec2(x, y);
        }
        default:
            ORIGIN_TYPE_ERROR("Rectangle", origin_type);
    }
}

static Vec2 get_triangle_origin(
    Triangle triangle, PrimitiveOriginType origin_type
) {
    switch (origin_type) {
        case BARYCENTRIC_ORIGIN: {
            Vec2 vertices[3];
            get_triangle_vertices(triangle, vertices);
            return scale(add_many(vertices, 3), 1.0 / 3.0);
        }
        default:
            ORIGIN_TYPE_ERROR("Triangle", origin_type);
    }
}

static Vec2 get_line_origin(Line line, PrimitiveOriginType origin_type) {
    switch (origin_type) {
        case BARYCENTRIC_ORIGIN:
            return scale(add(line.position, line.b), 0.5);
        default:
            ORIGIN_TYPE_ERROR("Line", origin_type);
    }
}

int get_primitive_vertices(Primitive primitive, Vec2* out) {
    PrimitiveType type = primitive.type;
    PrimitiveOriginType origin_type = BARYCENTRIC_ORIGIN;
    float rotation;
    Vec2 origin;
    int n;

    switch (type) {
        case CIRCLE_PRIMITIVE:
            return 0;
        case RECTANGLE_PRIMITIVE:
            rotation = primitive.p.rectangle.rotation;
            origin = get_rectangle_origin(
                primitive.p.rectangle, origin_type
            );
            n = get_rectangle_vertices(primitive.p.rectangle, out);
            break;
        case TRIANGLE_PRIMITIVE:
            rotation = primitive.p.triangle.rotation;
            origin = get_triangle_origin(
                primitive.p.triangle, origin_type
            );
            n = get_triangle_vertices(primitive.p.triangle, out);
            break;
        case LINE_PRIMITIVE:
            rotation = primitive.p.line.rotation;
            origin = get_line_origin(primitive.p.line, origin_type);
            n = get_line_vertices(primitive.p.line, out);
            break;
        default:
            PRIMITIVE_TYPE_ERROR("get_primitive_vertices", type);
    }

    for (int i = 0; i < n; ++i) {
        out[i] = rotate(out[i], origin, rotation);
    }
}

void translate_primitive(Primitive* primitive, Vec2 translation) {
    PrimitiveType type = primitive->type;
    switch (type) {
        case CIRCLE_PRIMITIVE:
            primitive->p.circle.position = add(
                primitive->p.circle.position, translation
            );
            break;
        case RECTANGLE_PRIMITIVE:
            primitive->p.rectangle.position = add(
                primitive->p.rectangle.position, translation
            );
            break;
        case TRIANGLE_PRIMITIVE:
            primitive->p.triangle.position = add(
                primitive->p.triangle.position, translation
            );
            break;
        case LINE_PRIMITIVE:
            primitive->p.line.position = add(
                primitive->p.line.position, translation
            );
            break;
        default:
            PRIMITIVE_TYPE_ERROR("translate_primitive", type);
    }
}

void rotate_primitive(Primitive* primitive, float angle) {
    PrimitiveType type = primitive->type;

    switch (type) {
        case CIRCLE_PRIMITIVE:
            primitive->p.circle.rotation += angle;
            break;
        case RECTANGLE_PRIMITIVE:
            primitive->p.rectangle.rotation += angle;
            break;
        case TRIANGLE_PRIMITIVE:
            primitive->p.triangle.rotation += angle;
            break;
        case LINE_PRIMITIVE:
            primitive->p.line.rotation += angle;
            break;
        default:
            PRIMITIVE_TYPE_ERROR("rotate_primitive", type);
    }
}
