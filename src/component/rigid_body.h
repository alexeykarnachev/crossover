#pragma once
#include "../math.h"

#define N_RIGID_BODY_TYPES 3
typedef enum RigidBodyType {
    STATIC_RIGID_BODY,
    KINEMATIC_RIGID_BODY,
    DYNAMIC_RIGID_BODY
} RigidBodyType;

extern RigidBodyType RIGID_BODY_TYPES[N_RIGID_BODY_TYPES];
extern const char *RIGID_BODY_TYPE_NAMES[N_RIGID_BODY_TYPES];

typedef struct StaticRigidBody {
} StaticRigidBody;

typedef struct KinematicRigidBody {
    Vec2 linear_velocity;
    float angular_velocity;
} KinematicRigidBody;

typedef struct DynamicRigidBody {
    Vec2 linear_velocity;
    float angular_velocity;
    Vec2 net_force;
    float net_torque;

    float mass;
    float linear_damping;

    float moment_of_inertia;
    float angular_damping;
    float angular_stiffness;
} DynamicRigidBody;

typedef struct RigidBody {
    RigidBodyType type;
    union {
        StaticRigidBody static_rb;
        KinematicRigidBody kinematic_rb;
        DynamicRigidBody dynamic_rb;
    } b;
} RigidBody;

void change_rigid_body_type(RigidBody *rigid_body, RigidBodyType target_type);

RigidBody init_static_rigid_body(void);
RigidBody init_kinematic_rigid_body(void);
RigidBody init_dynamic_rigid_body(
    float mass,
    float linear_damping,
    float moment_of_inertia,
    float angular_damping,
    float angular_stiffness
);

RigidBody init_default_static_rigid_body(void);
RigidBody init_default_kinematic_rigid_body(void);
RigidBody init_default_dynamic_rigid_body(void);

void apply_move_force_to_rb(RigidBody *rb, float move_orientation, float force_magnitude);
void apply_angular_torque_to_rb(RigidBody *rb, float orientation_diff);
