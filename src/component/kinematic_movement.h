#pragma once
#include "../math.h"

typedef struct KinematicMovement {
    Vec2 linear_velocity;

    float angular_velocity;

    float watch_orientation;
    float target_watch_orientation;
    int consider_target_watch_orientation;
} KinematicMovement;

KinematicMovement init_kinematic_movement(
    int consider_target_watch_orientation
);
