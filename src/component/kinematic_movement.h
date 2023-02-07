#pragma once
#include "../math.h"

typedef struct KinematicMovement {
    Vec2 move_dir;
    float speed;
    float target_orientation;
    int is_moving;
} KinematicMovement;

KinematicMovement init_kinematic_movement(
    Vec2 move_dir, float speed, float target_orientation, int is_moving
);
Vec2 get_kinematic_velocity(KinematicMovement movement);
