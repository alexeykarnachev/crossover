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
void update_collision();
void update_bullet();
void update_health();
void update_kinematic(float dt);
void update_player();
void update_ttl(float dt);
void update_vision();

RayCastResult cast_ray(
    Vec2 start, Vec2 ray, int target_components, int ray_owner
);
