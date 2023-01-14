#include "transformation.h"

#include "math.h"
#include "physics.h"
#include <math.h>
#include <stdio.h>

Transformation transformation(
    Vec2 position, Vec2 scale, float orientation
) {
    Transformation t = {position, scale, orientation};
    return t;
}

void update_transformation_by_physics(
    Transformation* t, Physics p, float dt
) {
    dt /= 1000.0;
    // float r = t->orientation + p.current_orientation;
    // Vec2 direction = {cos(r), -sin(r)};
    // float step_length = p.current_speed * dt / 1000.0;
    // Vec2 step = scale_vec2(direction, step_length);
    // t->position = add_vec2(t->position, step);
    if (length_vec2(p.movement_direction) > EPS) {
        Vec2 step = scale_vec2(
            normalize_vec2(p.movement_direction), p.movement_speed * dt
        );
        t->position = add_vec2(t->position, step);
    }
}
