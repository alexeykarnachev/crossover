#pragma once
#include "math.h"

typedef struct Kinematic {
    Vec2 velocity;
    float max_speed;
} Kinematic;

Kinematic kinematic(Vec2 velocity, float max_speed);
