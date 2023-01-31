#include "../debug.h"

#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../system.h"
#include "../world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Debug DEBUG;

void init_debug(void) {
    DEBUG.is_playing = 0;
    DEBUG.picked_entity.entity = -1;

    DEBUG.shading.materials = 1;
    DEBUG.shading.collisions = 1;
    DEBUG.shading.visions = 1;
    DEBUG.shading.kinematics = 1;
    DEBUG.shading.player = 1;
    DEBUG.collisions.resolve = 1;
}

void update_debug(void) {
    DEBUG.n_primitives = 0;
}

void render_debug_primitive(
    Transformation t, Primitive p, Vec3 color, int fill_type
) {
    if (DEBUG.n_primitives < MAX_N_DEBUG_PRIMITIVES) {
        DebugPrimitive* dp = &DEBUG.primitives[DEBUG.n_primitives++];
        dp->transformation = t;
        dp->primitive = p;
        dp->color = color;
        dp->fill_type = fill_type;
    } else {
        fprintf(stderr, "WARNING: Can't render more debug primitives\n");
    }
}

void render_debug_line(Vec2 s, Vec2 e, Vec3 color) {
    Vec2 d = sub(e, s);
    Transformation t = {add(s, scale(d, 0.5)), 0.0};
    Primitive p = init_line_primitive(d);
    render_debug_primitive(t, p, color, -1);
}

void render_debug_circle(Vec2 c, float r, Vec3 color, int fill_type) {
    Transformation t = {c, 0.0};
    Primitive p = init_circle_primitive(r);
    render_debug_primitive(t, p, color, fill_type);
}

void render_debug_rectangle(
    Vec2 position, float width, float height, Vec3 color, int fill_type
) {
    Transformation t = {position, 0.0};
    Primitive p = init_rectangle_primitive(width, height);
    render_debug_primitive(t, p, color, fill_type);
}

void render_debug_grid() {
    if (DEBUG.shading.grid && WORLD.camera != -1) {
        CameraFrustum frustum = get_camera_frustum();

        float x = ceilf(frustum.bot_left.x);
        while (x < frustum.top_right.x) {
            render_debug_line(
                vec2(x, frustum.bot_left.y),
                vec2(x, frustum.top_right.y),
                BLACK_COLOR
            );
            x += 1.0;
        }

        float y = ceilf(frustum.bot_left.y);
        while (y < frustum.top_right.y) {
            render_debug_line(
                vec2(frustum.bot_left.x, y),
                vec2(frustum.top_right.x, y),
                BLACK_COLOR
            );
            y += 1.0;
        }
    }
}
