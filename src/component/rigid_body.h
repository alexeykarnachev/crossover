#pragma once
#include "../math.h"

typedef struct RigidBody {
    int is_static;
    float mass;
    float restitution;
    Vec2 net_force;
    float net_torque;
    float linear_damping;
    float moment_of_inertia;
    float angular_stiffness;
    float angular_damping;
} RigidBody;

RigidBody init_rigid_body(
    int is_static,
    float mass,
    float restitution,
    float linear_damping,
    float moment_of_inertia,
    float angular_stiffness,
    float angular_damping
);
RigidBody init_default_rigid_body(int is_static);
RigidBody init_default_static_rigid_body(void);
RigidBody init_default_kinematic_rigid_body(void);
