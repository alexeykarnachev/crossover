#pragma once
#include "camera.h"
#include "const.h"
#include "transformation.h"
#include <stddef.h>

typedef struct World {
    size_t n_entities;

    // Components
    Transformation transformation[MAX_N_ENTITIES];
    Vec2 velocity[MAX_N_ENTITIES];
    int collider[MAX_N_ENTITIES];
    int circle[MAX_N_ENTITIES];

    Camera camera;
    int player;
} World;

extern World WORLD;

void init_world(void);
void update_world(void);
int spawn_guy(Transformation transformation);
