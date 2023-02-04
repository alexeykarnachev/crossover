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

void update_camera(void);
void update_collisions(void);
void update_bullets(float dt);
void update_healths(void);
void update_kinematics(float dt);
void update_player(void);
void update_ttls(float dt);
void update_visions(void);
void update_entity_picking(void);
void update_entity_dragging(void);

void render_debug_player(void);
void render_debug_kinematics(void);
void render_collision_mtvs(void);
void render_colliders(void);
void render_debug_visions(void);
void render_entity_handles(void);
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
Vec2 get_cursor_scene_pos(void);
int check_if_cursor_on_entity(int entity);
int get_entity_under_cursor(void);
