#pragma once
#include "camera.h"
#include "collision/collision.h"
#include "const.h"
#include "material.h"
#include "movement.h"
#include "primitive.h"
#include "vision.h"
#include <stddef.h>
#include <stdint.h>

typedef struct World {
    // Collisions
    Collision collisions[MAX_N_ENTITIES * MAX_N_ENTITIES];

    // Components
    uint64_t components[MAX_N_ENTITIES];
    Movement movement[MAX_N_ENTITIES];
    Vision vision[MAX_N_ENTITIES];
    Primitive collider[MAX_N_ENTITIES];
    Primitive primitive[MAX_N_ENTITIES];
    Material material[MAX_N_ENTITIES];
    int rectangle[MAX_N_ENTITIES];

    // Singleton entities
    Camera camera;
    int player;

    // Current counters
    size_t n_collisions;
    size_t n_entities;
} World;

typedef enum ComponentType {
    MOVEMENT_COMPONENT = 1 << 0,
    VISION_COMPONENT = 1 << 1,
    COLLIDER_COMPONENT = 1 << 2,
    RIGID_BODY_COMPONENT = 1 << 3,
    PRIMITIVE_COMPONENT = 1 << 4,
    MATERIAL_COMPONENT = 1 << 5
} ComponentType;

extern World WORLD;

void init_world(void);
int entity_has_component(int entity, ComponentType component);
int spawn_guy(
    Primitive primitive,
    Material material,
    Movement movement,
    Vision vision
);
int spawn_obstacle(Primitive primitive, Material material);
void update_world(float dt);
void transform_entity(int entity, Vec2 translation, float angle);
