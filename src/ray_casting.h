#pragma once
#include "math.h"
#include "primitive.h"

typedef struct RayCastResult {
    Vec2 position;
    int entity;
} RayCastResult;

RayCastResult cast_ray(
    Vec2 start, Line ray, int target_components, int ray_owner
);
