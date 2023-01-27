#pragma once
#include "./component/primitive.h"
#include "math.h"

typedef struct RayCastResult {
    Vec2 position;
    int entity;
} RayCastResult;

RayCastResult cast_ray(
    Vec2 start, Line ray, int target_components, int ray_owner
);
