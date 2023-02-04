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

typedef struct Line {
    Vec2 b;
} Line;

typedef struct Polygon {
    Vec2 vertices[MAX_N_POLYGON_VERTICES];
    int n_vertices;
} Polygon;

typedef enum PrimitiveType {
    CIRCLE_PRIMITIVE = 1 << 0,
    RECTANGLE_PRIMITIVE = 1 << 1,
    LINE_PRIMITIVE = 1 << 2,
    POLYGON_PRIMITIVE = 1 << 3
} PrimitiveType;

#define N_PRIMITIVE_TYPES 4
extern PrimitiveType PRIMITIVE_TYPES[N_PRIMITIVE_TYPES];

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
extern Vec3 REDWOOD_COLOR;
extern Vec3 GREEN_COLOR;
extern Vec3 BLUE_COLOR;
extern Vec3 BRIGHT_BLUE_COLOR;
extern Vec3 YELLOW_COLOR;
extern Vec3 LIGHT_YELLOW_COLOR;
extern Vec3 MAGENTA_COLOR;
extern Vec3 CYAN_COLOR;
extern Vec3 VIOLET_COLOR;
extern Vec3 WHITE_COLOR;
extern Vec3 BLACK_COLOR;
extern Vec3 GRAY_COLOR;
extern Vec3 DARK_GRAY_COLOR;
extern Vec3 ORANGE_COLOR;
extern Vec3 SILVER_COLOR;
extern Vec3 BROWN_COLOR;
extern Vec3 SKYBLUE_COLOR;

typedef enum ComponentType {
    TRANSFORMATION_COMPONENT = 1 << 0,
    COLLIDER_COMPONENT = 1 << 1,
    PRIMITIVE_COMPONENT = 1 << 2,
    RENDER_LAYER_COMPONENT = 1 << 3,
    MATERIAL_COMPONENT = 1 << 4,
    KINEMATIC_COMPONENT = 1 << 5,
    VISION_COMPONENT = 1 << 6,
    RIGID_BODY_COMPONENT = 1 << 7,
    OBSERVABLE_COMPONENT = 1 << 8,
    TTL_COMPONENT = 1 << 9,
    HEALTH_COMPONENT = 1 << 10,
    GUN_COMPONENT = 1 << 11,
    BULLET_COMPONENT = 1 << 12,
    OWNER_COMPONENT = 1 << 13
} ComponentType;

#define N_COMPONENTS 14
ComponentType COMPONENT_TYPES_LIST[N_COMPONENTS];
const char* get_component_type_name(ComponentType type);

typedef enum CompoundComponentType {
    RENDERABLE_COMPONENT = TRANSFORMATION_COMPONENT | PRIMITIVE_COMPONENT
                           | MATERIAL_COMPONENT | RENDER_LAYER_COMPONENT,
    CAN_OBSERVE_COMPONENT = TRANSFORMATION_COMPONENT | VISION_COMPONENT,
    CAN_COLLIDE_COMPONENT = TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT,
    KINEMATIC_BULLET_COMPONENT = TRANSFORMATION_COMPONENT
                                 | BULLET_COMPONENT | KINEMATIC_COMPONENT
                                 | TTL_COMPONENT,
    DAMAGEABLE_BY_BULLET_COMPONENT = TRANSFORMATION_COMPONENT
                                     | COLLIDER_COMPONENT
                                     | RIGID_BODY_COMPONENT
} CompoundComponentType;

Kinematic init_kinematic(float max_speed, float rotation_speed);
float get_kinematic_damage(Kinematic kinematic);

Gun init_gun(float bullet_ttl, float bullet_speed, float fire_rate);

Material init_material(Vec3 diffuse_color);

Transformation init_transformation(Vec2 position, float orientation);
void apply_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
);
void apply_inverse_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
);

Vision init_vision(float fov, float distance, int n_view_rays);
void reset_vision(Vision* v);
