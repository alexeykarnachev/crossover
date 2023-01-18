#include "debug.h"

#include "../const.h"
#include "../world.h"
#include <stdio.h>
#include <stdlib.h>

Debug DEBUG;

void init_debug(void) {
    DEBUG.shading.material = 1;
    DEBUG.shading.collision_axis = 1;
}

void update_debug(void) {
    DEBUG.general.n_entities = WORLD.n_entities;
    DEBUG.general.n_collisions = WORLD.n_collisions;
}

void submit_debug_render_command(RenderCommand command) {
    if (DEBUG.n_render_commands < MAX_N_DEBUG_RENDER_COMMANDS) {
        DEBUG.render_commands[DEBUG.n_render_commands++] = command;
    }
}
