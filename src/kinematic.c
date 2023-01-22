#include "kinematic.h"

#include "math.h"

Kinematic kinematic(Vec2 velocity, float max_speed) {
    Kinematic k = {velocity, max_speed};
    return k;
}
