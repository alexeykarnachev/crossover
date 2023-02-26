#pragma once
#include "../math.h"

typedef struct KinematicMovement {
    float mass;
    Vec2 linear_velocity;
    Vec2 net_force;
    float linear_damping;

    float moment_of_inertia;
    float angular_velocity;
    float net_torque;
    float angular_damping;
    float angular_stiffness;

    float watch_orientation;
    float target_watch_orientation;
} KinematicMovement;

KinematicMovement init_kinematic_movement(
    float mass,
    float linear_damping,
    float moment_of_inertia,
    float angular_damping,
    float angular_stiffness
);
