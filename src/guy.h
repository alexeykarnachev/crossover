#pragma once
#include "math.h"

typedef struct Guy {
    float size;
    float health;
    float score;
    float movement_speed;
    float rotation_speed;

    float n_view_rays;
    float view_angle;
    float view_distance;

    float rotation;
    Vec2 position;
} Guy;
