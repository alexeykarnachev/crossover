#pragma once
#include "math.h"

typedef struct Transformation {
    Vec2 position;
    Vec2 scale;
    float orientation;
} Transformation;

Transformation transformation(
    Vec2 position, Vec2 scale, float orientation
);
