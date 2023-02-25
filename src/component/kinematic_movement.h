#pragma once
#include "../math.h"

typedef struct KinematicMovement {
    Vec2 velocity;
    Vec2 acceleration;
    float max_speed;
    float friction_coefficient;
    float acceleration_scalar;

    float watch_orientation;
} KinematicMovement;

KinematicMovement init_kinematic_movement(
    float max_speed, float friction_coefficient, float acceleration_scalar
);
