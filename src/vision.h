#pragma once
#include "math.h"

typedef struct Vision {
    Vec2 position;
    float rotation;

    float fov;
    float distance;
    int n_view_rays;
} Vision;

Vision vision(
    Vec2 position,
    float rotation,
    float fov,
    float distance,
    int n_view_rays
);

void translate_vision(Vision* vision, Vec2 translation);
void rotate_vision(Vision* vision, float rotation);
void get_vision_rays(Vision vision, Vec2* out);

void observe_another_entity(int e0, int e1);
