#include "movement.h"

#include "math.h"

Movement movement(
    float max_speed, float acceleration, float rotation_speed
) {
    Movement m = {
        0.0, 0.0, 0.0, 0.0, max_speed, acceleration, rotation_speed};
    return m;
}

void update_movement(Movement* m, float dt) {
    dt /= 1000.0;

    // Update current speed
    if (m->target_speed < m->current_speed) {
        m->current_speed -= m->acceleration * dt;
    } else {
        m->current_speed += m->acceleration * dt;
    }
    m->current_speed = clamp(m->current_speed, 0.0, m->max_speed);

    // Update orientation
    float step = m->rotation_speed * dt;
    float diff = get_orientations_diff(
        m->target_orientation, m->current_orientation
    );
    if (diff <= step) {
        m->current_orientation = m->target_orientation;
    } else {
        float new_r0 = m->current_orientation + step;
        float new_r1 = m->current_orientation - step;
        float diff0 = get_orientations_diff(m->target_orientation, new_r0);
        float diff1 = get_orientations_diff(m->target_orientation, new_r1);
        m->current_orientation = diff0 < diff1 ? new_r0 : new_r1;
    }
}
