#pragma once

#include "const.h"
#include "math.h"

typedef struct Transformation {
    Vec2 position;
    float orientation;
} Transformation;

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
Rectangle get_primitive_bounding_rectangle(
    Primitive primitive, Transformation transformation
);

typedef struct RayCastResult {
    Vec2 position;
    int entity;
} RayCastResult;

typedef struct Vision {
    RayCastResult observations[MAX_N_VIEW_RAYS];

    float fov;
    float distance;
    int n_view_rays;
} Vision;

typedef struct Gun {
    struct bullet {
        float ttl;
        float speed;
    } bullet;

    float fire_rate;
    float last_time_shoot;
} Gun;

typedef struct Kinematic {
    Vec2 velocity;
    float max_speed;
    float orientation;
    float rotation_speed;
} Kinematic;

typedef struct Material {
    Vec3 diffuse_color;
} Material;

extern Vec3 RED_COLOR;
extern Vec3 GREEN_COLOR;
extern Vec3 BLUE_COLOR;
extern Vec3 YELLOW_COLOR;
extern Vec3 MAGENTA_COLOR;
extern Vec3 CYAN_COLOR;
extern Vec3 VIOLET_COLOR;
extern Vec3 WHITE_COLOR;
extern Vec3 BLACK_COLOR;
extern Vec3 GRAY_COLOR;
extern Vec3 ORANGE_COLOR;
extern Vec3 SILVER_COLOR;
extern Vec3 BROWN_COLOR;
extern Vec3 SKYBLUE_COLOR;

Kinematic init_kinematic(
    Vec2 velocity, float max_speed, float rotation_speed
);
float get_kinematic_damage(Kinematic kinematic);

Gun init_gun(float bullet_ttl, float bullet_speed, float fire_rate);

Material init_material(Vec3 diffuse_color);

Transformation init_transformation(Vec2 position, float orientation);
void apply_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
);

Vision init_vision(float fov, float distance, int n_view_rays);
void reset_vision(Vision* v);
