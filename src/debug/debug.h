#pragma once
#include "../const.h"
#include "../renderer.h"

typedef struct Debug {
    RenderCommand render_commands[MAX_N_DEBUG_RENDER_COMMANDS];
    int n_render_commands;

    struct general {
        int n_entities;
        int n_collisions;
    } general;

    struct shading {
        int material;
        int wireframe;
        int mtv;
        int collision_axis;
    } shading;

    struct collisions {
        int resolve;
        int resolve_once;
    } collisions;
} Debug;

extern Debug DEBUG;

void init_debug(void);
void update_debug(void);
void submit_debug_render_command(RenderCommand render_command);
