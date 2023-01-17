#include "primitive.h"

#include "math.h"
#include "movement.h"
#include <stdio.h>

Circle circle(Vec2 position, float radius) {
    Circle circle = {position, radius};
    return circle;
}

Rectangle rectangle(Vec2 position, float width, float height) {
    Rectangle rectangle = {position, width, height};
    return rectangle;
}

Triangle triangle(Vec2 position, Vec2 b, Vec2 c) {
    Triangle triangle = {position, b, c};
    return triangle;
}

Line line(Vec2 position, Vec2 b) {
    Line line = {position, b};
    return line;
}

Primitive circle_primitive(Vec2 position, float radius) {
    Primitive primitive;
    primitive.type = CIRCLE_PRIMITIVE;
    primitive.p.circle = circle(position, radius);
    return primitive;
}

Primitive rectangle_primitive(Vec2 position, float width, float height) {
    Primitive primitive;
    primitive.type = RECTANGLE_PRIMITIVE;
    primitive.p.rectangle = rectangle(position, width, height);
    return primitive;
}

Primitive triangle_primitive(Vec2 position, Vec2 b, Vec2 c) {
    Primitive primitive;
    primitive.type = TRIANGLE_PRIMITIVE;
    primitive.p.triangle = triangle(position, b, c);
    return primitive;
}

Primitive line_primitive(Vec2 position, Vec2 b) {
    Primitive primitive;
    primitive.type = LINE_PRIMITIVE;
    primitive.p.line = line(position, b);
    return primitive;
}

void get_triangle_vertices(Triangle triangle, Vec2* out) {
    Vec2 a = triangle.position;
    Vec2 b = add_vec2(a, triangle.b);
    Vec2 c = add_vec2(a, triangle.c);
    out[0] = a;
    out[1] = b;
    out[2] = c;
}

void get_rectangle_vertices(Rectangle rectangle, Vec2* out) {
    Vec2 d = rectangle.position;
    Vec2 a = {d.x, d.y + rectangle.height};
    Vec2 b = {a.x + rectangle.width, a.y};
    Vec2 c = {b.x, d.y};

    out[0] = a;
    out[1] = b;
    out[2] = c;
    out[3] = d;
}

Vec2 get_primitive_position(Primitive primitive) {
    PrimitiveType type = primitive.type;
    if (type == CIRCLE_PRIMITIVE) {
        return primitive.p.circle.position;
    } else if (type == RECTANGLE_PRIMITIVE) {
        return primitive.p.rectangle.position;
    } else if (type == TRIANGLE_PRIMITIVE) {
        return primitive.p.triangle.position;
    } else if (type == LINE_PRIMITIVE) {
        return primitive.p.line.position;
    } else {
        fprintf(
            stderr,
            "ERROR: can't get the position of the primitive with type id: "
            "%d. Needs to be implemented\n",
            type
        );
    }
}

void translate_primitive(Primitive* primitive, Vec2 translation) {
    if (primitive->type & CIRCLE_PRIMITIVE) {
        primitive->p.circle.position = add_vec2(
            primitive->p.circle.position, translation
        );
    } else if (primitive->type & RECTANGLE_PRIMITIVE) {
        primitive->p.rectangle.position = add_vec2(
            primitive->p.rectangle.position, translation
        );
    } else if (primitive->type & TRIANGLE_PRIMITIVE) {
        primitive->p.triangle.position = add_vec2(
            primitive->p.triangle.position, translation
        );
    } else if (primitive->type & LINE_PRIMITIVE) {
        primitive->p.line.position = add_vec2(
            primitive->p.line.position, translation
        );
    }
}
