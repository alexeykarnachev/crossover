#pragma once
#include "collision.h"
#include "const.h"
#include "gun.h"
#include "kinematic.h"
#include "material.h"
#include "primitive.h"
#include "ttl.h"
#include "vision.h"
#include <stddef.h>
#include <stdint.h>

typedef struct World {
    // Collisions
    Collision collisions[MAX_N_ENTITIES * MAX_N_ENTITIES];

    // Components
    uint64_t components[MAX_N_ENTITIES];
    Transformation transformation[MAX_N_ENTITIES];
    Kinematic kinematic[MAX_N_ENTITIES];
    Vision vision[MAX_N_ENTITIES];
    Primitive collider[MAX_N_ENTITIES];
    Primitive primitive[MAX_N_ENTITIES];
    Material material[MAX_N_ENTITIES];
    Gun gun[MAX_N_ENTITIES];
    TTL ttl[MAX_N_ENTITIES];

    // Singleton entities
    int camera;
    int player;

    // Current counters
    size_t n_collisions;
    size_t n_entities;
    size_t n_destroyed_entities;
} World;

typedef enum ComponentType {
    TRANSFORMATION_COMPONENT = 1 << 0,
    KINEMATIC_COMPONENT = 1 << 1,
    VISION_COMPONENT = 1 << 2,
    COLLIDER_COMPONENT = 1 << 3,
    RIGID_BODY_COMPONENT = 1 << 4,
    PRIMITIVE_COMPONENT = 1 << 5,
    MATERIAL_COMPONENT = 1 << 6,
    OBSERVABLE_COMPONENT = 1 << 7,
    GUN_COMPONENT = 1 << 8,
    TTL_COMPONENT = 1 << 9
} ComponentType;

extern World WORLD;

typedef struct CameraFrustum {
    Vec2 bot_left;
    Vec2 top_right;
} CameraFrustum;

CameraFrustum get_camera_frustum();

void init_world(void);
int entity_has_component(int entity, ComponentType component);
int entity_has_ttl(int entity);
int entity_can_collide(int entity);
int entity_can_observe(int entity);
int entity_can_apply_kinematic(int entity);
int entity_can_be_rendered(int entity);
int entity_can_be_observed(int entity);
int spawn_camera(Transformation transformation);
int spawn_player(
    Transformation transformation,
    Primitive primitive,
    Material material,
    Kinematic kinematic,
    Vision vision
);
int spawn_guy(
    Transformation transformation,
    Primitive primitive,
    Material material,
    Kinematic kinematic,
    Vision vision
);
int spawn_obstacle(
    Transformation transformation, Primitive primitive, Material material
);
int spawn_bullet(
    Transformation transformation,
    Primitive primitive,
    Material material,
    Kinematic kinematic,
    TTL ttl
);
void set_gun(int entity, Gun gun);
void update_world(float dt);
void transform_entity(int entity, Vec2 translation, float angle);
