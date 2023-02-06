#pragma once
#include "../const.h"
#include "../math.h"
#include "transformation.h"

typedef enum PrimitiveType {
    CIRCLE_PRIMITIVE = 1 << 0,
    RECTANGLE_PRIMITIVE = 1 << 1,
    LINE_PRIMITIVE = 1 << 2,
    POLYGON_PRIMITIVE = 1 << 3
} PrimitiveType;

#define N_PRIMITIVE_TYPES 4
extern PrimitiveType PRIMITIVE_TYPES[N_PRIMITIVE_TYPES];

typedef struct Circle {
    float radius;
} Circle;

typedef struct Rectangle {
    float width;
    float height;
} Rectangle;

typedef struct Line {
    Vec2 b;
} Line;

typedef struct Polygon {
    Vec2 vertices[MAX_N_POLYGON_VERTICES];
    int n_vertices;
} Polygon;

typedef struct Primitive {
    PrimitiveType type;
    union {
        Circle circle;
        Rectangle rectangle;
        Line line;
        Polygon polygon;
    } p;
} Primitive;

const char* get_primitive_type_name(PrimitiveType type);
void change_primitive_type(
    Primitive* primitive, PrimitiveType target_type
);
Primitive init_circle_primitive(float radius);
Primitive init_rectangle_primitive(float width, float height);
Primitive init_line_primitive(Vec2 b);
Primitive init_polygon_primitive(
    Vec2 vertices[MAX_N_POLYGON_VERTICES], int n_vertices
);
Primitive init_default_circle_primitive(void);
Primitive init_default_rectangle_primitive(void);
Primitive init_default_line_primitive(void);
Primitive init_default_polygon_primitive(void);
int get_primitive_vertices(Primitive primitive, Vec2* out);
int get_primitive_fan_vertices(
    Primitive primitive, Vec2 out[MAX_N_POLYGON_VERTICES]
);
Rectangle get_primitive_bounding_rectangle(
    Primitive primitive, Transformation transformation
);

void add_polygon_vertex(Polygon* polygon);
void delete_polygon_vertex(Polygon* polygon);
