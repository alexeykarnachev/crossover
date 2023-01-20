#pragma once
#include "math.h"

typedef struct Movement {
    Vec2 direction;
    float speed;
    float rotation_speed;
} Movement;

Movement movement(float speed, float rotation_speed);
void move_entity(int entity, float dt);
