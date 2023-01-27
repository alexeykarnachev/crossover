#pragma once
#include "math.h"

typedef struct RayCastResult {
    Vec2 position;
    int entity;
} RayCastResult;

RayCastResult cast_ray(
    Vec2 start, Vec2 direction, int target_components, int ray_owner
);
