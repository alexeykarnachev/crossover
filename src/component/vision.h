#pragma once
#include "../const.h"
#include "../math.h"

typedef struct RayCastResult {
    Vec2 position;
    int entity;
} RayCastResult;

typedef struct Vision {
    RayCastResult observations[MAX_N_VIEW_RAYS];

    float fov;
    float distance;
    int n_view_rays;
} Vision;

Vision init_vision(float fov, float distance, int n_view_rays);
void reset_vision(Vision *vision);
