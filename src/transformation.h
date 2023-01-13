#pragma once
#include "math.h"
#include "movement.h"

typedef struct Transformation {
    Vec2 position;
    Vec2 scale;
    float orientation;
} Transformation;

Transformation transformation(
    Vec2 position, Vec2 scale, float orientation
);

void update_transformation_by_movement(
    Transformation* t, Movement m, float dt
);
