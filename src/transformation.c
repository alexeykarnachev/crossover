#include "transformation.h"

#include "math.h"
#include <math.h>
#include <stdio.h>

Transformation transformation(
    Vec2 position, Vec2 scale, float orientation
) {
    Transformation t = {position, scale, orientation};
    return t;
}

void update_transformation_by_movement(
    Transformation* t, Movement m, float dt
) {
    float r = t->orientation + m.current_orientation;
    Vec2 direction = {cos(r), -sin(r)};
    float step_length = m.current_speed * dt / 1000.0;
    Vec2 step = scale_vec2(direction, step_length);
    t->position = add_vec2(t->position, step);
}
