#pragma once
#include "../const.h"
#include "../math.h"
#include "transformation.h"

#define N_PRIMITIVE_TYPES 4
typedef enum PrimitiveType {
    CIRCLE_PRIMITIVE,
    RECTANGLE_PRIMITIVE,
    LINE_PRIMITIVE,
    POLYGON_PRIMITIVE
} PrimitiveType;

extern PrimitiveType PRIMITIVE_TYPES[N_PRIMITIVE_TYPES];
const char* PRIMITIVE_TYPE_NAMES[N_PRIMITIVE_TYPES];

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
void get_unit_circle_fan_vertices(Vec2* vertices, int n_vertices);
int get_primitive_vertices(Primitive primitive, Vec2* vertices);
int get_primitive_fan_vertices(
    Primitive primitive, Vec2 vertices[MAX_N_POLYGON_VERTICES]
);
void get_vertex_uvs(
    Vec2 vertices[MAX_N_POLYGON_VERTICES],
    int n,
    Vec2 uvs[MAX_N_POLYGON_VERTICES],
    Vec2* uv_size
);

void add_polygon_vertex(Polygon* polygon);
void delete_polygon_vertex(Polygon* polygon);
