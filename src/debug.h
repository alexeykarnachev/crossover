#pragma once
#include "component.h"
#include "const.h"
#include "math.h"
#include "scene.h"

typedef struct DebugPrimitive {
    Transformation transformation;
    Primitive primitive;
    Vec3 color;
    float render_layer;
    int fill_type;
} DebugPrimitive;

typedef struct Debug {
    DebugPrimitive primitives[MAX_N_DEBUG_PRIMITIVES];

    int n_primitives;

    struct general {
        Vec2 camera_position;
    } general;

    struct shading {
        int materials;
        int lights;
        int visions;
        int wireframe;
        int grid;

        float grid_tile_size;
    } shading;

    struct collisions {
        int n_update_steps;
        int resolve;
        int resolve_once;
    } collisions;

    struct gameplay {
        int all_immortal;
        int speed;
    } gameplay;
} Debug;

extern Debug DEBUG;

void init_debug(void);
void update_debug(void);

void render_debug_primitive(
    Transformation t, Primitive p, Vec3 color, float render_layer, int fill_type
);
void render_debug_line(Vec2 s, Vec2 e, Vec3 color, float render_layer);
void render_debug_circle(Vec2 c, float r, Vec3 color, float render_layer, int fill_type);
void render_debug_rectangle(
    Vec2 position,
    float width,
    float height,
    Vec3 color,
    float render_layer,
    int fill_type
);
void render_debug_grid(float tile_size);
