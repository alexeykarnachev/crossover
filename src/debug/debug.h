#pragma once
#include "../const.h"
#include "../renderer.h"

typedef struct DebugPrimitive {
    Transformation transformation;
    Primitive primitive;
    Material material;
} DebugPrimitive;

typedef struct Debug {
    DebugPrimitive primitives[MAX_N_DEBUG_PRIMITIVES];
    int n_primitives;

    struct general {
        int n_entities;
        int n_destroyed_entities;
        int n_collisions;
        Vec2 camera_position;
        Vec2 look_at;
    } general;

    struct shading {
        int materials;
        int wireframe;
        int collisions;
        int vision;
        int grid;
        int look_at;
        int orientation;
    } shading;

    struct collisions {
        int resolve;
        int resolve_once;
    } collisions;

    struct inputs {
        float cursor_x;
        float cursor_y;
        float cursor_dx;
        float cursor_dy;
    } inputs;
} Debug;

extern Debug DEBUG;

void init_debug(void);
void update_debug(void);
void render_debug_primitive(Transformation t, Primitive p, Material m);
void render_debug_line(Vec2 s, Vec2 e, Vec3 color);
void render_debug_circle(Vec2 c, float r, Vec3 color);
