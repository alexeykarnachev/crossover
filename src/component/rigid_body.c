#include "rigid_body.h"

#include <stdio.h>
#include <stdlib.h>

#define RIGID_BODY_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the Rigid body with type id: %d. Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

RigidBodyType RIGID_BODY_TYPES[N_RIGID_BODY_TYPES] = {
    STATIC_BODY, KINEMATIC_BODY};
const char* RIGID_BODY_TYPE_NAMES[N_RIGID_BODY_TYPES] = {
    "Static", "Kinematic"};

void change_rigid_body_type(
    RigidBody* rigid_body, RigidBodyType target_type
) {
    RigidBodyType source_type = rigid_body->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case STATIC_BODY:
            *rigid_body = init_default_static_rigid_body();
            break;
        case KINEMATIC_BODY:
            *rigid_body = init_default_kinematic_rigid_body();
            break;
        default:
            RIGID_BODY_TYPE_ERROR("change_rigid_body_type", source_type);
    }
}

RigidBody init_static_rigid_body(void) {
    RigidBody body;
    body.type = STATIC_BODY;
    return body;
}

RigidBody init_kinematic_rigid_body(void) {
    RigidBody body;
    body.type = KINEMATIC_BODY;
    return body;
}

RigidBody init_default_static_rigid_body(void) {
    return init_static_rigid_body();
}

RigidBody init_default_kinematic_rigid_body(void) {
    return init_kinematic_rigid_body();
}
