#pragma once
#include "camera.h"
#include "const.h"
#include "physics.h"
#include "primitive.h"
#include <stddef.h>
#include <stdint.h>

typedef struct World {
    size_t n_entities;

    // Components
    uint64_t components[MAX_N_ENTITIES];

    Physics physics[MAX_N_ENTITIES];
    Primitive primitive[MAX_N_ENTITIES];
    int collider[MAX_N_ENTITIES];
    int rectangle[MAX_N_ENTITIES];

    Camera camera;
    int player;
} World;

typedef enum ComponentType {
    PHYSICS_COMPONENT = 1 << 0,
    COLLIDER_COMPONENT = 1 << 1,
    PRIMITIVE_COMPONENT = 1 << 2
} ComponentType;

extern World WORLD;

void init_world(void);
int entity_has_component(int entity, ComponentType component);
int spawn_guy(Primitive primitive, Physics physics);
int spawn_obstacle(Primitive primitive);
void update_world(float dt);
