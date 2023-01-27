#pragma once
#include "math.h"

// TODO: Hold the collisions array in this file, not in the world file

typedef struct Collision {
    Vec2 mtv;
    int entity0;
    int entity1;
} Collision;

void update_collision();
void update_bullet();
void update_health();
void update_kinematic(float dt);
void update_player();
void update_ttl(float dt);
void update_vision();
