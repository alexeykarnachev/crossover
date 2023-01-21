#include "vision.h"

#include "collision.h"
#include "debug/debug.h"
#include "math.h"
#include "renderer.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>

static Vec2 VIEW_RAYS_ARENA[MAX_N_VIEW_RAYS];

Vision vision(
    Vec2 position,
    float rotation,
    float fov,
    float distance,
    int n_view_rays
) {
    if (n_view_rays > MAX_N_VIEW_RAYS) {
        exit(1);
    }
    Vision v = {position, rotation, fov, distance, n_view_rays};
    return v;
}

void translate_vision(Vision* vision, Vec2 translation) {
    vision->position = add(vision->position, translation);
}

void rotate_vision(Vision* vision, float angle) {
    vision->rotation += angle;
}

static int get_view_rays(Vision vision, Vec2* out) {
    Vec2 origin = {0.0, 0.0};
    Vec2 start_ray = {vision.distance, 0.0};

    float step;
    if (vision.n_view_rays == 1) {
        step = 0.0;
    } else {
        step = vision.fov / (vision.n_view_rays - 1);
    }

    for (int i = 0; i < vision.n_view_rays; ++i) {
        float angle = -vision.rotation - 0.5 * vision.fov + i * step;
        out[i] = rotate(start_ray, origin, angle);
    }

    return vision.n_view_rays;
}

void observe_another_entity(int e0, int e1) {
    int can_observe = entity_has_component(e0, VISION_COMPONENT)
                      && entity_has_component(e1, COLLIDER_COMPONENT);
    if (!can_observe) {
        return;
    }

    Collision collision;
    Vision v = WORLD.vision[e0];
    Primitive collider = WORLD.collider[e1];

    int n_view_rays = get_view_rays(v, VIEW_RAYS_ARENA);
    for (int i = 0; i < n_view_rays; ++i) {
        Line ray = line(v.position, VIEW_RAYS_ARENA[i], 0.0);
        int can_see = collide_primitives(
            line_primitive(ray), collider, &collision
        );
        if (can_see) {
            submit_debug_render_command(
                render_line_command(ray, material(RED_COLOR))
            );
        }
    }
}
