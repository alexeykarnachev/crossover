#include "kinematic_movement.h"

#include "../math.h"
#include <stdio.h>

KinematicMovement init_kinematic_movement(
    float friction_coefficient, float acceleration_scalar
) {
    KinematicMovement movement = {0};
    movement.friction_coefficient = friction_coefficient;
    movement.acceleration_scalar = acceleration_scalar;
    return movement;
}
