#include "vision.h"

#include "math.h"
#include "world.h"

Vision vision(
    Vec2 position,
    float rotation,
    float fov,
    float distance,
    int n_view_rays
) {
    Vision v = {position, rotation, fov, distance, n_view_rays};
    return v;
}

void translate_vision(Vision* vision, Vec2 translation) {
    vision->position = add(vision->position, translation);
}

void rotate_vision(Vision* vision, float angle) {
    vision->rotation += angle;
}

void get_vision_rays(Vision vision, Vec2* out) {
    Vec2 origin = vision.position;
    Vec2 start_ray = {origin.x + vision.distance, origin.y};

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
}

void observe_another_entity(int e0, int e1) {
    int can_observe = entity_has_component(e0, VISION_COMPONENT)
                      && entity_has_component(e1, COLLIDER_COMPONENT);
    if (!can_observe) {
        return;
    }
}
