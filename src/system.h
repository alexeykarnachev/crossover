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

void update_camera();
void update_collisions();
void update_bullets();
void update_healths();
void update_kinematics(float dt);
void update_player();
void update_ttls(float dt);
void update_visions();

void render_debug_player();
void render_debug_kinematics();
void render_debug_collisions();
void render_debug_visions();

RayCastResult cast_ray(
    Vec2 start, Vec2 ray, int target_components, int ray_owner
);
