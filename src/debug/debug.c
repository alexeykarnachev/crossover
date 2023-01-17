#include "debug.h"

#include "../world.h"
#include <stdio.h>
#include <stdlib.h>

Debug DEBUG;

void init_debug(void) {
    DEBUG.shading.material = 1;
}

void update_debug(void) {
    DEBUG.general.n_entities = WORLD.n_entities;
    DEBUG.general.n_collisions = WORLD.n_collisions;
}
