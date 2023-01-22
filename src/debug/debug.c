#include "debug.h"

#include "../app.h"
#include "../const.h"
#include "../world.h"
#include <stdio.h>
#include <stdlib.h>

Debug DEBUG;

void init_debug(void) {
    DEBUG.shading.materials = 1;
    DEBUG.shading.collisions = 1;
    DEBUG.shading.vision = 1;
    DEBUG.collisions.resolve = 1;
}

void update_debug(void) {
    DEBUG.general.n_entities = WORLD.n_entities;
    DEBUG.general.n_collisions = WORLD.n_collisions;
    DEBUG.inputs.cursor_x = APP.cursor_x;
    DEBUG.inputs.cursor_y = APP.cursor_y;
    DEBUG.inputs.cursor_dx = APP.cursor_dx;
    DEBUG.inputs.cursor_dy = APP.cursor_dy;
}
