#include "rigid_body.h"

#include <stdio.h>
#include <stdlib.h>

RigidBody init_rigid_body(
    int is_static,
    float mass,
    float restitution,
    float linear_damping,
    float moment_of_inertia,
    float angular_stiffness,
    float angular_damping
) {
    RigidBody body = {
        .is_static = is_static,
        .mass = mass,
        .restitution = restitution,
        .linear_damping = linear_damping,
        .moment_of_inertia = moment_of_inertia,
        .angular_stiffness = angular_stiffness,
        .angular_damping = angular_damping};
    return body;
}

RigidBody init_default_rigid_body(int is_static) {
    RigidBody body = {
        .is_static = is_static,
        .mass = 100.0,
        .restitution = 0.5,
        .linear_damping = 500.0,
        .moment_of_inertia = 0.01,
        .angular_stiffness = 1.0,
        .angular_damping = 0.1};
    return body;
}

RigidBody init_default_static_rigid_body(void) {
    return init_default_rigid_body(1);
}

RigidBody init_default_kinematic_rigid_body(void) {
    return init_default_rigid_body(0);
}
