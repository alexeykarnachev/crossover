#include "kinematic_movement.h"

#include "../math.h"
#include <stdio.h>

KinematicMovement init_kinematic_movement(
    Vec2 move_dir, float speed, float target_orientation, int is_moving
) {
    KinematicMovement movement;
    movement.move_dir = move_dir;
    movement.speed = speed;
    movement.target_orientation = target_orientation;
    movement.is_moving = is_moving;
    return movement;
}

Vec2 get_kinematic_velocity(KinematicMovement movement) {
    if (length(movement.move_dir) < EPS) {
        return vec2(0.0, 0.0);
    }

    return scale(normalize(movement.move_dir), movement.speed);
}
