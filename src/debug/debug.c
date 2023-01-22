#include "debug.h"

#include "../const.h"
#include "../world.h"
#include "../app.h"
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

void submit_debug_render_command(RenderCommand command) {
    if (DEBUG.n_render_commands < MAX_N_DEBUG_RENDER_COMMANDS) {
        DEBUG.render_commands[DEBUG.n_render_commands++] = command;
    }
}
