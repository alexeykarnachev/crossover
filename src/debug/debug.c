#include "debug.h"

#include "../world.h"

Debug DEBUG;

void init_debug(void) {}

void update_debug(void) {
    DEBUG.n_collisions.touch = 0;
    DEBUG.n_collisions.intersection = 0;
    DEBUG.n_collisions.containment = 0;

    for (int i = 0; i < WORLD.n_collisions; ++i) {
        if (WORLD.collisions[i].type & TOUCH_COLLISION) {
            DEBUG.n_collisions.touch += 1;
        } else if (WORLD.collisions[i].type & INTERSECTION_COLLISION) {
            DEBUG.n_collisions.intersection += 1;
        } else if (WORLD.collisions[i].type & (CONTAINMENT_COLLISION_0 | CONTAINMENT_COLLISION_1)) {
            DEBUG.n_collisions.containment += 1;
        }
    }
}
