#include "kinematic_movement.h"

KinematicMovement init_kinematic_movement(
    int consider_target_watch_orientation
) {
    KinematicMovement movement = {0};
    movement.consider_target_watch_orientation
        = consider_target_watch_orientation;
    return movement;
}
