#include "movement.h"

#include "math.h"

Movement movement(float speed, float rotation_speed) {
    Movement m = {vec2(0.0, 0.0), speed, rotation_speed};
    return m;
}
