#pragma once
#include "../math.h"

typedef struct KinematicMovement {
    float speed;
    float watch_orientation;
    float move_orientation;
    int is_moving;
} KinematicMovement;

KinematicMovement init_kinematic_movement(
    float speed,
    float watch_orientation,
    float move_orientation,
    int is_moving
);
Vec2 get_kinematic_velocity(KinematicMovement movement);
