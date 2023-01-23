#include "kinematic.h"

#include "math.h"

Kinematic kinematic(Vec2 velocity, float max_speed, float rotation_speed) {
    Kinematic k = {velocity, max_speed, 0.0, rotation_speed};
    return k;
}
