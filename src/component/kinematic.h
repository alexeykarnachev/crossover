#pragma once
#include "../math.h"

typedef struct Kinematic {
    Vec2 velocity;
    float max_speed;
    float orientation;
    float rotation_speed;
} Kinematic;

Kinematic init_kinematic(float max_speed, float rotation_speed);
float get_kinematic_damage(Kinematic kinematic);
