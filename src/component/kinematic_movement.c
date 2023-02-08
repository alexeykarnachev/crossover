#include "kinematic_movement.h"

#include "../math.h"
#include <stdio.h>

KinematicMovement init_kinematic_movement(
    float speed,
    float watch_orientation,
    float move_orientation,
    int is_moving
) {
    KinematicMovement movement;
    movement.speed = speed;
    movement.watch_orientation = watch_orientation;
    movement.move_orientation = move_orientation;
    movement.is_moving = is_moving;
    return movement;
}

Vec2 get_kinematic_velocity(KinematicMovement movement) {
    if (!movement.is_moving) {
        return vec2(0.0, 0.0);
    }
    Vec2 move_dir = get_orientation_vec(movement.move_orientation);
    return scale(normalize(move_dir), movement.speed);
}
