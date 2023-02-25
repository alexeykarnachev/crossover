#include "kinematic_movement.h"

#include "../math.h"
#include <stdio.h>

KinematicMovement init_kinematic_movement(
    float mass,
    float linear_damping,
    float moment_of_inertia,
    float angular_damping,
    float angular_stiffness
) {
    KinematicMovement movement = {0};
    movement.mass = mass;
    movement.linear_damping = linear_damping;
    movement.moment_of_inertia = moment_of_inertia;
    movement.angular_damping = angular_damping;
    movement.angular_stiffness = angular_stiffness;
    return movement;
}
