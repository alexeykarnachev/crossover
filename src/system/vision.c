#include "vision.h"

#include "../component/component.h"
#include "../debug/debug.h"
#include "../world.h"

static Vec2 VIEW_RAYS_ARENA[MAX_N_VIEW_RAYS];

static int get_view_rays(Vision vision, float orientation, Vec2* out) {
    Vec2 origin = {0.0, 0.0};
    Vec2 start_ray = rotate(
        vec2(vision.distance, 0.0), origin, orientation
    );

    float step;
    if (vision.n_view_rays == 1) {
        step = 0.0;
    } else {
        step = vision.fov / (vision.n_view_rays - 1);
    }

    for (int i = 0; i < vision.n_view_rays; ++i) {
        float angle = -0.5 * vision.fov + i * step;
        out[i] = rotate(start_ray, origin, angle);
    }

    return vision.n_view_rays;
}

void update_vision() {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_can_observe(entity)) {
            continue;
        }

        Vision v = WORLD.vision[entity];
        Transformation t0 = WORLD.transformation[entity];
        Vec2 start = t0.position;
        reset_observations(&v);
        int n_view_rays = get_view_rays(
            v, t0.orientation, VIEW_RAYS_ARENA
        );

        for (int i = 0; i < n_view_rays; ++i) {
            Line ray = {VIEW_RAYS_ARENA[i]};
            RayCastResult observation = cast_ray(
                start, ray, OBSERVABLE_COMPONENT, entity
            );
            v.observations[i] = observation;
        }

        if (DEBUG.shading.vision) {
            for (int i = 0; i < v.n_view_rays; ++i) {
                RayCastResult obs = v.observations[i];
                if (obs.entity != -1) {
                    render_debug_line(
                        t0.position, obs.position, GREEN_COLOR
                    );
                    render_debug_circle(obs.position, 0.05, RED_COLOR, -1);
                } else {
                    render_debug_line(
                        t0.position,
                        add(t0.position, VIEW_RAYS_ARENA[i]),
                        GREEN_COLOR
                    );
                }
            }
        }
    }
}
