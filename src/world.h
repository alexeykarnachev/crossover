#pragma once
#include "component.h"
#include "const.h"
#include "system.h"
#include <stddef.h>
#include <stdint.h>

typedef struct World {
    // Components
    uint64_t components[MAX_N_ENTITIES];
    Transformation transformations[MAX_N_ENTITIES];
    Kinematic kinematics[MAX_N_ENTITIES];
    Vision visions[MAX_N_ENTITIES];
    Primitive colliders[MAX_N_ENTITIES];
    Primitive primitives[MAX_N_ENTITIES];
    Material materials[MAX_N_ENTITIES];
    Gun guns[MAX_N_ENTITIES];
    float ttls[MAX_N_ENTITIES];
    float healths[MAX_N_ENTITIES];
    int owners[MAX_N_ENTITIES];

    // Singleton entities
    int camera;
    int player;

    // Current counters
    size_t n_entities;
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
    TTL_COMPONENT = 1 << 9,
    BULLET_COMPONENT = 1 << 10,
    HEALTH_COMPONENT = 1 << 11,
    OWNER_COMPONENT = 1 << 12
} ComponentType;

extern World WORLD;

typedef struct CameraFrustum {
    Vec2 bot_left;
    Vec2 top_right;
} CameraFrustum;

CameraFrustum get_camera_frustum();

void init_world(void);

void destroy_entity(int entity);
int get_entity_owner(int entity);

int entity_has_component(int entity, ComponentType component);
int entity_has_transformation(int entity);
int entity_has_primitive(int entity);
int entity_has_collider(int entity);
int entity_has_ttl(int entity);
int entity_has_rigid_body(int entity);
int entity_has_bullet(int entity);
int entity_has_health(int entity);
int entity_has_kinematic(int entity);
int entity_has_owner(int entity);

int entity_can_collide(int entity);
int entity_can_observe(int entity);
int entity_can_be_rendered(int entity);
int entity_can_be_damaged_by_bullet(int entity, int bullet);
int bullet_can_be_destroyed_after_collision(int bullet, int target);

int spawn_camera(Transformation transformation);
int spawn_guy(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material,
    Kinematic kinematic,
    Vision vision,
    Gun gun,
    float health,
    int is_player
);
int spawn_obstacle(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material
);
int spawn_bullet(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material,
    Kinematic kinematic,
    float ttl,
    int owner
);
void update_world(float dt);
