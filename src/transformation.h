#pragma once
#include "math.h"

typedef struct Transformation {
    Vec2 position;
    float rotation;
} Transformation;

Transformation transformation(Vec2 position, float rotation);
Vec2 transform(Vec2 v, Transformation t);
