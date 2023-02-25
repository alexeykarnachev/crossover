#pragma once
#include "../math.h"

// TODO: Add kinematic movement rotation inertia
typedef struct KinematicMovement {
    Vec2 velocity;
    Vec2 acceleration;
    float friction_coefficient;
    float acceleration_scalar;

    float watch_orientation;
} KinematicMovement;

KinematicMovement init_kinematic_movement(
    float friction_coefficient, float acceleration_scalar
);
