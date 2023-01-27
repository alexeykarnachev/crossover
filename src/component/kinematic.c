#include "kinematic.h"

#include "../debug/debug.h"
#include "../math.h"
#include "../world.h"
#include "material.h"
#include "transformation.h"
#include <math.h>

Kinematic kinematic(Vec2 velocity, float max_speed, float rotation_speed) {
    Kinematic k = {velocity, max_speed, 0.0, rotation_speed};
    return k;
}

float get_kinematic_damage(Kinematic kinematic) {
    return length(kinematic.velocity);
}
