#pragma once
#include "math.h"
#include "movement.h"

typedef struct Circle {
    Vec2 position;
    float radius;
    float rotation;
} Circle;

typedef struct Rectangle {
    Vec2 position;
    float width;
    float height;
    float rotation;
} Rectangle;

typedef struct Triangle {
    Vec2 position;
    Vec2 b;
    Vec2 c;
    float rotation;
} Triangle;

typedef struct Line {
    Vec2 position;
    Vec2 b;
    float rotation;
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

Circle circle(Vec2 position, float radius, float rotation);
Rectangle rectangle(
    Vec2 position, float width, float height, float rotation
);
Triangle triangle(Vec2 position, Vec2 b, Vec2 c, float rotation);
Line line(Vec2 position, Vec2 b, float rotation);
Primitive circle_primitive(Vec2 position, float radius, float rotation);
Primitive rectangle_primitive(
    Vec2 position, float width, float height, float rotation
);
Primitive triangle_primitive(
    Vec2 position, Vec2 b, Vec2 c, float rotation
);
Primitive line_primitive(Vec2 position, Vec2 b, float rotation);

int get_primitive_vertices(Primitive primitive, Vec2* out);
void translate_primitive(Primitive* primitive, Vec2 translation);
void rotate_primitive(Primitive* primitive, float angle);
