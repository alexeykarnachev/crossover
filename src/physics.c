#include "physics.h"

#include "math.h"

Physics physics(float movement_speed, float rotation_speed) {
    Physics p = {vec2(0.0, 0.0), movement_speed, rotation_speed};
    return p;
}
