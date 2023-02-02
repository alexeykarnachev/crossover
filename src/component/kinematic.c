#include "../component.h"
#include "../math.h"

Kinematic init_kinematic(float max_speed, float rotation_speed) {
    Kinematic k = {vec2(0.0, 0.0), max_speed, 0.0, rotation_speed};
    return k;
}

float get_kinematic_damage(Kinematic kinematic) {
    return length(kinematic.velocity);
}
