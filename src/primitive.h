#pragma once
#include "math.h"

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

typedef struct Primitive {
    PrimitiveType type;
    union {
        Circle circle;
        Rectangle rectangle;
        Triangle triangle;
        Line line;
    } p;
} Primitive;

Primitive init_circle_primitive(float radius);
Primitive init_rectangle_primitive(float width, float height);
Primitive init_triangle_primitive(Vec2 b, Vec2 c);
Primitive init_line_primitive(Vec2 b);
int get_primitive_vertices(Primitive primitive, Vec2* out);
