#pragma once
#include "asset.h"
#include "component.h"
#include "const.h"
#include "math.h"

typedef enum BrainFitsEntityErrorReason {
    VISION_COMPONENT_MISSED_ERROR,
    HEALTH_COMPONENT_MISSED_ERROR,
    RIGID_BODY_COMPONENT_MISSED_ERROR,
    STATIC_RIGID_BODY_COMPONENT_ERROR,
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
void update_bullets(float dt);
void update_healths(float dt);
void update_rigid_bodies(float dt);
void update_ttls(float dt);
void update_visions(void);
void update_controllers(void);

void update_do_kill_score(int entity);
void update_do_move_score(int entity, Vec2 step);
void update_do_rotation_score(int entity, float angular_step);
void update_do_shoot_score(int entity);
void update_do_hit_score(int entity);
void update_get_killed_score(int entity);
void update_get_hit_score(int entity);
void update_exploration_score(int entity, float dt);

void render_debug_player(void);
void render_debug_visions(void);
void render_bullets(float dt);

RayCastResult cast_ray(
    Vec2 start, Vec2 ray, int target_components, int ray_owner
);
