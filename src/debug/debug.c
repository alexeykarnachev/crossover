#include "debug.h"

#include "../app.h"
#include "../const.h"
#include "../primitive.h"
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

void render_debug_primitive(Transformation t, Primitive p, Material m) {
    if (DEBUG.n_primitives < MAX_N_DEBUG_PRIMITIVES) {
        DebugPrimitive* dp = &DEBUG.primitives[DEBUG.n_primitives++];
        dp->transformation = t;
        dp->primitive = p;
        dp->material = m;
    }
}

void render_debug_line(Vec2 s, Vec2 e, Vec3 color) {
    Vec2 d = sub(e, s);
    Transformation t = {add(s, scale(d, 0.5)), 0.0};
    Primitive p = line_primitive(line(d));
    Material m = {color};
    render_debug_primitive(t, p, m);
}

void render_debug_circle(Vec2 c, float r, Vec3 color) {
    Transformation t = {c, 0.0};
    Primitive p = circle_primitive(circle(r));
    Material m = {color};
    render_debug_primitive(t, p, m);
}
