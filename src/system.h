#pragma once
#include "component.h"
#include "const.h"
#include "math.h"

typedef struct Collision {
    Vec2 mtv;
    int entity0;
    int entity1;
} Collision;

typedef struct CollisionsArena {
    Collision arena[MAX_N_COLLISIONS];
    int n;
} CollisionsArena;

typedef enum BrainFitsEntityErrorReason {
    VISION_COMPONENT_MISSED_ERROR,
    HEALTH_COMPONENT_MISSED_ERROR,
    N_VIEW_RAYS_MISSMATCH_ERROR
} BrainFitsEntityErrorReason;

typedef struct BrainFitsEntityError {
    BrainFitsEntityErrorReason reasons[4];
    int n_reasons;
} BrainFitsEntityError;

BrainFitsEntityError check_if_brain_fits_entity(
    BrainParams params, int entity
);

void update_camera(void);
void update_collisions(void);
void update_bullets(float dt);
void update_healths(void);
void update_kinematic_movements(float dt);
void update_ttls(float dt);
void update_visions(void);
void update_controllers(void);

void render_debug_player(void);
void render_debug_kinematic_movements(void);
void render_collision_mtvs(void);
void render_colliders(void);
void render_debug_visions(void);
void render_bullets(float dt);

int collide_primitives(
    Primitive primitive0,
    Transformation transformation0,
    Primitive primitive1,
    Transformation transformation1,
    Collision* collision
);
RayCastResult cast_ray(
    Vec2 start, Vec2 ray, int target_components, int ray_owner
);
