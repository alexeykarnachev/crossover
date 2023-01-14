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
    Vec2 a;
    Vec2 b;
    Vec2 c;
} Triangle;

typedef enum PrimitiveType {
    CIRCLE_PRIMITIVE = 1 << 0,
    RECTANGLE_PRIMITIVE = 1 << 1,
    TRIANGLE_PRIMITIVE = 1 << 2
} PrimitiveType;

typedef struct Primitive {
    PrimitiveType type;
    union {
        Circle circle;
        Rectangle rectangle;
        Triangle triangle;
    } p;
} Primitive;

Circle circle(Vec2 position, float radius);
Rectangle rectangle(Vec2 position, float width, float height);
Triangle triangle(Vec2 a, Vec2 b, Vec2 c);
Primitive circle_primitive(Vec2 position, float radius);
Primitive rectangle_primitive(Vec2 position, float width, float height);
Primitive triangle_primitive(Vec2 a, Vec2 b, Vec2 c);

void move_primitive(Primitive* primitive, Movement movement, float dt);
