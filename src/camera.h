#pragma once
#include "math.h"

typedef struct Camera {
    Vec2 position;
    float elevation;
} Camera;

Camera camera(Vec2 position, float elevation);
