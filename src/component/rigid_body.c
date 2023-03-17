#include "rigid_body.h"

#include <stdio.h>
#include <stdlib.h>

#define RIGID_BODY_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the RigidBody with type id: %d. Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

RigidBodyType RIGID_BODY_TYPES[N_RIGID_BODY_TYPES] = {
    STATIC_RIGID_BODY, KINEMATIC_RIGID_BODY, DYNAMIC_RIGID_BODY};

const char* RIGID_BODY_TYPE_NAMES[N_RIGID_BODY_TYPES] = {
    "Static", "Kinematic", "Dynamic"};

void change_rigid_body_type(
    RigidBody* rigid_body, RigidBodyType target_type
) {
    RigidBodyType source_type = rigid_body->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case STATIC_RIGID_BODY:
            *rigid_body = init_default_static_rigid_body();
            break;
        case KINEMATIC_RIGID_BODY:
            *rigid_body = init_default_kinematic_rigid_body();
            break;
        case DYNAMIC_RIGID_BODY:
            *rigid_body = init_default_dynamic_rigid_body();
            break;
        default:
            RIGID_BODY_TYPE_ERROR("change_rigid_body_type", source_type);
    }
}

RigidBody init_static_rigid_body(void) {
    RigidBody rb = {0};
    rb.type = STATIC_RIGID_BODY;
    return rb;
}

RigidBody init_kinematic_rigid_body(void) {
    RigidBody rb = {0};
    rb.type = KINEMATIC_RIGID_BODY;
    return rb;
}

RigidBody init_dynamic_rigid_body(
    float mass,
    float linear_damping,
    float moment_of_inertia,
    float angular_damping,
    float angular_stiffness
) {
    RigidBody rb = {0};
    rb.type = DYNAMIC_RIGID_BODY;
    rb.b.dynamic_rb.mass = mass;
    rb.b.dynamic_rb.linear_damping = linear_damping;
    rb.b.dynamic_rb.moment_of_inertia = moment_of_inertia;
    rb.b.dynamic_rb.angular_damping = angular_damping;
    rb.b.dynamic_rb.angular_stiffness = angular_stiffness;
    return rb;
}

RigidBody init_default_static_rigid_body(void) {
    return init_static_rigid_body();
}

RigidBody init_default_kinematic_rigid_body(void) {
    return init_kinematic_rigid_body();
}

RigidBody init_default_dynamic_rigid_body(void) {
    float mass = 70.0;
    float linear_damping = 1.0;
    float moment_of_inertia = 1.0;
    float angular_damping = 1.0;
    float angular_stiffness = 1.0;
    return init_dynamic_rigid_body(
        mass,
        linear_damping,
        moment_of_inertia,
        angular_damping,
        angular_stiffness
    );
}

void apply_move_force_to_rb(
    RigidBody* rb, float move_orientation, float force_magnitude
) {
    if (rb->type != DYNAMIC_RIGID_BODY) {
        fprintf(
            stderr,
            "ERROR: Can't apply move force to the non-dynamic rigid body\n"
        );
        exit(1);
    }
    Vec2 force = scale(
        get_orientation_vec(move_orientation), force_magnitude
    );
    rb->b.dynamic_rb.net_force = add(rb->b.dynamic_rb.net_force, force);
}

void apply_angular_torque_to_rb(RigidBody* rb, float orientation_diff) {
    if (rb->type != DYNAMIC_RIGID_BODY) {
        fprintf(
            stderr,
            "ERROR: Can't apply angular torque to the non-dynamic rigid "
            "body\n"
        );
        exit(1);
    }
    float torque = orientation_diff * rb->b.dynamic_rb.angular_stiffness;
    rb->b.dynamic_rb.net_torque += torque;
}
