#pragma once
#include "../const.h"
#include "../ray_casting.h"

typedef struct Vision {
    RayCastResult observations[MAX_N_VIEW_RAYS];

    float fov;
    float distance;
    int n_view_rays;
} Vision;

Vision vision(float fov, float distance, int n_view_rays);
void reset_observations(Vision* v);
