#pragma once
#include "math.h"

typedef struct Movement {
    Vec2 current_velocity;
    float target_speed;
    float max_speed;
    float acceleration;
    float rotation_speed;
} Movement;

Movement movement(
    float max_speed, float acceleration, float rotation_speed
);

void update_movement(Movement* m, float dt);
