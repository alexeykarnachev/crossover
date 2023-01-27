#pragma once
#include "../math.h"
#include "transformation.h"

typedef struct Circle {
    float radius;
} Circle;

typedef struct Rectangle {
    float width;
    float height;
} Rectangle;

typedef struct Triangle {
    Vec2 b;
    Vec2 c;
} Triangle;

typedef struct Line {
    Vec2 b;
} Line;

typedef enum PrimitiveType {
    CIRCLE_PRIMITIVE = 1 << 0,
    RECTANGLE_PRIMITIVE = 1 << 1,
    TRIANGLE_PRIMITIVE = 1 << 2,
    LINE_PRIMITIVE = 1 << 3
} PrimitiveType;

typedef enum PrimitiveOriginType {
    BARYCENTRIC_ORIGIN = 1 << 0
} PrimitiveOriginType;

typedef struct Primitive {
    PrimitiveType type;
    union {
        Circle circle;
        Rectangle rectangle;
        Triangle triangle;
        Line line;
    } p;
} Primitive;

Circle circle(float radius);
Rectangle rectangle(float width, float height);
Triangle triangle(Vec2 b, Vec2 c);
Line line(Vec2 b);
Primitive circle_primitive(Circle c);
Primitive rectangle_primitive(Rectangle r);
Primitive triangle_primitive(Triangle t);
Primitive line_primitive(Line v);

int get_primitive_vertices(
    Primitive primitive, Transformation transformation, Vec2* out
);
