#include "movement.h"

#include "math.h"
#include "world.h"

Movement movement(float speed, float rotation_speed) {
    Movement m = {vec2(0.0, 0.0), speed, 0.0, rotation_speed};
    return m;
}

