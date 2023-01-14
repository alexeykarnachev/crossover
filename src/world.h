#pragma once
#include "camera.h"
#include "const.h"
#include "physics.h"
#include "transformation.h"
#include <stddef.h>
#include <stdint.h>

typedef struct World {
    size_t n_entities;

    // Components
    uint64_t components[MAX_N_ENTITIES];
    Transformation transformation[MAX_N_ENTITIES];
    Physics physics[MAX_N_ENTITIES];
    int collider[MAX_N_ENTITIES];
    int circle[MAX_N_ENTITIES];

    Camera camera;
    int player;
} World;

typedef enum Component {
    TRANSFORMATION_COMPONENT = 1,
    PHYSICS_COMPONENT,
    COLLIDER_COMPONENT,
    CIRCLE_COMPONENT
} Component;

extern World WORLD;

void init_world(void);
int entity_has_component(int entity, Component component);
int spawn_guy(Transformation t, Physics p);
void update_world(float dt);
