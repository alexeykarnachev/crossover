#pragma once
#include "math.h"

typedef struct Physics {
    Vec2 movement_direction;
    float movement_speed;
    float rotation_speed;
} Physics;

Physics physics(float movement_speed, float rotation_speed);
