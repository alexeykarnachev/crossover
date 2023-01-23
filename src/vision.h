#pragma once
#include "const.h"
#include "math.h"

typedef struct Observation {
    Vec2 position;
    int entity;
} Observation;

typedef struct Vision {
    Observation observations[MAX_N_VIEW_RAYS];

    float fov;
    float distance;
    int n_view_rays;
} Vision;

Observation observation(Vec2 position, int entity);
Vision vision(float fov, float distance, int n_view_rays);
void reset_observations(Vision* v);

void observe_world(int entity);
