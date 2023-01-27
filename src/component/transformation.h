#pragma once
#include "../math.h"

typedef struct Transformation {
    Vec2 position;
    float orientation;
} Transformation;

Transformation transformation(Vec2 position, float orientation);
Vec2 transform(Vec2 v, Transformation t);
