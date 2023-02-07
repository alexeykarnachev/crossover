#pragma once

typedef enum RigidBodyType {
    STATIC_BODY = 1 << 0,
    KINEMATIC_BODY = 1 << 1
} RigidBodyType;

#define N_RIGID_BODY_TYPES 2
extern RigidBodyType RIGID_BODY_TYPES[N_RIGID_BODY_TYPES];

typedef struct StaticRigidBody {
} StaticRigidBody;

typedef struct KinematicRigidBody {
} KinematicRigidBody;

typedef struct RigidBody {
    RigidBodyType type;
    union {
        StaticRigidBody static_body;
        KinematicRigidBody kinematic_body;
    } b;
} RigidBody;

const char* get_rigid_body_type_name(RigidBodyType type);
void change_rigid_body_type(
    RigidBody* rigid_body, RigidBodyType target_type
);
RigidBody init_static_rigid_body(void);
RigidBody init_kinematic_rigid_body(void);
RigidBody init_default_static_rigid_body(void);
RigidBody init_default_kinematic_rigid_body(void);
