#pragma once
#include "component.h"
#include "const.h"
#include "math.h"

typedef struct DebugPrimitive {
    Transformation transformation;
    Primitive primitive;
    Vec3 color;
    int fill_type;
} DebugPrimitive;

typedef struct Debug {
    DebugPrimitive primitives[MAX_N_DEBUG_PRIMITIVES];
    int n_primitives;
    int is_playing;
    int picked_entity;
    int is_dragging;

    struct general {
        int n_entities;
        int n_collisions;
        Vec2 camera_position;
        Vec2 look_at;
    } general;

    struct shading {
        int materials;
        int collisions;
        int kinematics;
        int visions;
        int player;
        int wireframe;
        int grid;
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
        float scroll_dy;
    } inputs;
} Debug;

extern Debug DEBUG;

void init_debug(void);
void update_debug(void);
void update_debug_gui(void);

void render_debug_primitive(
    Transformation t, Primitive p, Vec3 color, int fill_type
);
void render_debug_line(Vec2 s, Vec2 e, Vec3 color);
void render_debug_circle(Vec2 c, float r, Vec3 color, int fill_type);
void render_debug_rectangle(
    Vec2 position, float width, float height, Vec3 color, int fill_type
);
void render_debug_gui(void);
void render_debug_grid(void);
