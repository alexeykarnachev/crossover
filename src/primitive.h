#pragma once
#include "math.h"
#include "movement.h"

typedef struct Circle {
    Vec2 position;
    float radius;
} Circle;

typedef struct Rectangle {
    Vec2 position;
    float width;
    float height;
} Rectangle;

typedef struct Triangle {
    Vec2 position;
    Vec2 b;
    Vec2 c;
} Triangle;

typedef struct Line {
    Vec2 position;
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

Circle circle(Vec2 position, float radius);
Rectangle rectangle(Vec2 position, float width, float height);
Triangle triangle(Vec2 position, Vec2 b, Vec2 c);
Line line(Vec2 position, Vec2 b);
Primitive circle_primitive(Vec2 position, float radius);
Primitive rectangle_primitive(Vec2 position, float width, float height);
Primitive triangle_primitive(Vec2 position, Vec2 b, Vec2 c);
Primitive line_primitive(Vec2 position, Vec2 b);

void get_triangle_vertices(Triangle triangle, Vec2* out);
void get_rectangle_vertices(Rectangle rectangle, Vec2* out);

Vec2 get_primitive_position(Primitive primitive);
void translate_primitive(Primitive* primitive, Vec2 translation);
