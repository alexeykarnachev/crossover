#pragma once
#include "math.h"

typedef struct Kinematic {
    Vec2 velocity;
    float max_speed;
    float orientation;
    float rotation_speed;
} Kinematic;

Kinematic kinematic(Vec2 velocity, float max_speed, float rotation_speed);
void apply_kinematic(int entity, float dt);
float get_kinematic_damage(Kinematic kinematic);
