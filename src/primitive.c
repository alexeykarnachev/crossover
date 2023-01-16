#include "primitive.h"

#include "math.h"
#include "movement.h"

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

Line line(Vec2 a, Vec2 b) {
    Line line = {a, b};
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

Primitive line_primitive(Vec2 a, Vec2 b) {
    Primitive primitive;
    primitive.type = LINE_PRIMITIVE;
    primitive.p.line = line(a, b);
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

void move_primitive(Primitive* primitive, Movement movement, float dt) {
    dt /= 1000.0;
    if (length_vec2(movement.direction) > EPS) {
        Vec2 step = scale_vec2(
            normalize_vec2(movement.direction), movement.speed * dt
        );
        if (primitive->type & CIRCLE_PRIMITIVE) {
            primitive->p.circle.position = add_vec2(
                primitive->p.circle.position, step
            );
        } else if (primitive->type & RECTANGLE_PRIMITIVE) {
            primitive->p.rectangle.position = add_vec2(
                primitive->p.rectangle.position, step
            );
        } else if (primitive->type & TRIANGLE_PRIMITIVE) {
            primitive->p.triangle.position = add_vec2(
                primitive->p.triangle.position, step
            );
        } else if (primitive->type & LINE_PRIMITIVE) {
            primitive->p.line.a = add_vec2(primitive->p.line.a, step);
            primitive->p.line.b = add_vec2(primitive->p.line.b, step);
        }
    }
}
