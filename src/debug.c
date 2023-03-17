#include "debug.h"

#include "app.h"
#include "component.h"
#include "const.h"
#include "gl.h"
#include "scene.h"
#include "system.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Debug DEBUG = {0};

void init_debug(void) {
    DEBUG.shading.materials = 1;

    DEBUG.collisions.resolve = 1;
    DEBUG.collisions.n_update_steps = 1;

    DEBUG.shading.grid_tile_size = SCENE_TILE_SIZE;

    DEBUG.gameplay.speed = 1;
}

void update_debug(void) {
    DEBUG.n_primitives = 0;
}

void render_debug_primitive(
    Transformation t,
    Primitive p,
    Vec3 color,
    float render_layer,
    int fill_type
) {
    if (DEBUG.n_primitives < MAX_N_DEBUG_PRIMITIVES) {
        DebugPrimitive* dp = &DEBUG.primitives[DEBUG.n_primitives++];
        dp->transformation = t;
        dp->primitive = p;
        dp->color = color;
        dp->render_layer = render_layer;
        dp->fill_type = fill_type;
    } else {
        fprintf(stderr, "WARNING: Can't render more debug primitives\n");
    }
}

void render_debug_line(Vec2 s, Vec2 e, Vec3 color, float render_layer) {
    Vec2 d = sub(e, s);
    Transformation t = init_transformation(add(s, scale(d, 0.5)), 0.0);
    Primitive p = init_line_primitive(d);
    render_debug_primitive(t, p, color, render_layer, FILL);
}

void render_debug_circle(
    Vec2 c, float r, Vec3 color, float render_layer, int fill_type
) {
    Transformation t = init_transformation(c, 0.0);
    Primitive p = init_circle_primitive(r);
    render_debug_primitive(t, p, color, render_layer, fill_type);
}

void render_debug_rectangle(
    Vec2 position,
    float width,
    float height,
    Vec3 color,
    float render_layer,
    int fill_type
) {
    Transformation t = init_transformation(position, 0.0);
    Primitive p = init_rectangle_primitive(width, height);
    render_debug_primitive(t, p, color, render_layer, fill_type);
}

void render_debug_grid(float cell_size) {
    cell_size = max(cell_size, 1.0);
    CameraFrustum frustum = get_camera_frustum();
    float frustum_width = frustum.top_right.x - frustum.bot_left.x;
    float frustum_height = frustum.top_right.y - frustum.bot_left.y;
    Vec2 expand = sub(frustum.top_right, frustum.bot_left);
    frustum.bot_left = sub(frustum.bot_left, expand);
    frustum.top_right = add(frustum.top_right, expand);

    float offset = N_SCENE_TILES % 2 == 0 ? 0.0 : 0.5;

    float x = frustum.bot_left.x;
    float x_start = cell_size * (offset + floor(abs(x) / cell_size));
    x = x <= 0 ? -x_start : x_start;
    while (x < frustum.top_right.x) {
        render_debug_line(
            vec2(x, frustum.bot_left.y),
            vec2(x, frustum.top_right.y),
            BLACK_COLOR,
            DEBUG_RENDER_LAYER
        );
        x += cell_size;
    }

    float y = frustum.bot_left.y;
    float y_start = cell_size * (offset + floor(abs(y) / cell_size));
    y = y <= 0 ? -y_start : y_start;
    while (y < frustum.top_right.y) {
        render_debug_line(
            vec2(frustum.bot_left.x, y),
            vec2(frustum.top_right.x, y),
            BLACK_COLOR,
            DEBUG_RENDER_LAYER
        );
        y += cell_size;
    }
}
