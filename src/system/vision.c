#include "../component.h"
#include "../debug.h"
#include "../gl.h"
#include "../system.h"
#include "../world.h"

static Vec2 VIEW_RAYS_ARENA[MAX_N_VIEW_RAYS];

static int get_view_rays(Vision vision, float orientation, Vec2* out) {
    Vec2 origin = {0.0, 0.0};
    Vec2 start_ray = rotate(
        vec2(vision.distance, 0.0), origin, orientation
    );

    if (vision.n_view_rays == 1) {
        out[0] = start_ray;
    } else {
        float step = vision.fov / (vision.n_view_rays - 1);
        for (int i = 0; i < vision.n_view_rays; ++i) {
            float angle = -0.5 * vision.fov + i * step;
            out[i] = rotate(start_ray, origin, angle);
        }
    }

    return vision.n_view_rays;
}

void update_visions() {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, CAN_OBSERVE_COMPONENT)) {
            continue;
        }

        Vision* vision = &WORLD.visions[entity];
        Transformation t0 = WORLD.transformations[entity];
        Vec2 start = t0.position;
        reset_vision(vision);
        int n_view_rays = get_view_rays(
            *vision, t0.orientation, VIEW_RAYS_ARENA
        );

        for (int i = 0; i < n_view_rays; ++i) {
            RayCastResult observation = cast_ray(
                start, VIEW_RAYS_ARENA[i], OBSERVABLE_COMPONENT, entity
            );
            vision->observations[i] = observation;
        }
    }
}

void render_debug_visions() {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, CAN_OBSERVE_COMPONENT)) {
            continue;
        }

        Vision vision = WORLD.visions[entity];
        Transformation t0 = WORLD.transformations[entity];
        int n_view_rays = get_view_rays(
            vision, t0.orientation, VIEW_RAYS_ARENA
        );

        for (int i = 0; i < n_view_rays; ++i) {
            RayCastResult observation = vision.observations[i];
            if (observation.entity != -1) {
                render_debug_line(
                    t0.position,
                    observation.position,
                    GREEN_COLOR,
                    DEBUG_RENDER_LAYER
                );
                render_debug_circle(
                    observation.position,
                    0.05,
                    RED_COLOR,
                    DEBUG_RENDER_LAYER,
                    FILL
                );
            } else {
                render_debug_line(
                    t0.position,
                    add(t0.position, VIEW_RAYS_ARENA[i]),
                    GREEN_COLOR,
                    DEBUG_RENDER_LAYER
                );
            }
        }
    }
}
