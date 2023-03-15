#pragma once
#include "../math.h"

typedef struct Transformation {
    Vec2 prev_position;
    Vec2 curr_position;
    float prev_orientation;
    float curr_orientation;
} Transformation;

Transformation init_transformation(Vec2 position, float orientation);
void apply_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
);
void apply_inverse_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
);
