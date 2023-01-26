#include "vision.h"

#include "debug/debug.h"
#include "math.h"
#include "renderer.h"
#include "transformation.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Vec2 VIEW_RAYS_ARENA[MAX_N_VIEW_RAYS];

Observation observation(Vec2 position, int entity) {
    Observation obs = {position, entity};
    return obs;
}

Vision vision(float fov, float distance, int n_view_rays) {
    if (n_view_rays > MAX_N_VIEW_RAYS) {
        fprintf(
            stderr,
            "ERROR: can't create the Vision component with more than %d "
            "view rays\n",
            MAX_N_VIEW_RAYS
        );
        exit(1);
    }

    Vision v;
    v.fov = fov;
    v.distance = distance;
    v.n_view_rays = n_view_rays;

    reset_observations(&v);
    return v;
}

void reset_observations(Vision* v) {
    memset(v->observations, -1, sizeof(Observation) * v->n_view_rays);
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
        float angle = -0.5 * vision.fov + i * step;
        out[i] = rotate(start_ray, origin, angle);
    }

    return vision.n_view_rays;
}

static int intersect_line_with_primitive_nearest(
    Transformation t0,
    Line line,
    Transformation t1,
    Primitive primitive,
    Vec2* out
) {
    switch (primitive.type) {
        case CIRCLE_PRIMITIVE:
            return intersect_line_with_circle_nearest(
                transform(vec2(0.0, 0.0), t0),
                transform(line.b, t0),
                t1.position,
                primitive.p.circle.radius,
                out
            );
        default: {
            Vec2 vertices[4];
            int nv = get_primitive_vertices(primitive, t1, vertices);
            Vec2 foo = transform(line.b, t0);
            return intersect_line_with_polygon_nearest(
                t0.position, transform(line.b, t0), vertices, nv, out
            );
        }
    }
}

void observe_world(int entity) {
    if (!entity_can_observe(entity)) {
        return;
    }

    Vision v = WORLD.vision[entity];
    Transformation t0 = WORLD.transformation[entity];
    reset_observations(&v);
    int n_view_rays = get_view_rays(v, VIEW_RAYS_ARENA);
    for (int target = 0; target < WORLD.n_entities; ++target) {
        int can_be_observed = target != entity
                              && entity_can_be_observed(target);
        if (!can_be_observed) {
            continue;
        }

        Transformation t1 = WORLD.transformation[target];
        Primitive collider = WORLD.collider[target];
        for (int i = 0; i < n_view_rays; ++i) {
            Line ray = {VIEW_RAYS_ARENA[i]};
            Observation obs;
            int is_observed = intersect_line_with_primitive_nearest(
                t0, ray, t1, collider, &obs.position
            );
            if (!is_observed) {
                continue;
            }

            float new_dist = dist(t0.position, obs.position);
            float old_dist = dist(t0.position, v.observations[i].position);
            int is_best = v.observations[i].entity == -1
                          || new_dist < old_dist;
            if (is_best) {
                obs.entity = target;
                v.observations[i] = obs;
            }
        }
    }

    if (DEBUG.shading.vision) {
        for (int i = 0; i < v.n_view_rays; ++i) {
            Observation obs = v.observations[i];
            if (obs.entity != -1) {
                render_debug_line(t0.position, obs.position, GREEN_COLOR);
                render_debug_circle(obs.position, 0.05, RED_COLOR);
            } else {
                render_debug_line(
                    t0.position,
                    transform(VIEW_RAYS_ARENA[i], t0),
                    GREEN_COLOR
                );
            }
        }
    }
}
