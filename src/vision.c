#include "vision.h"

#include "debug/debug.h"
#include "math.h"
#include "renderer.h"
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

    Vision v;
    reset_observations(&v);
    v.position = position;
    v.rotation = rotation;
    v.fov = fov;
    v.distance = distance;
    v.n_view_rays = n_view_rays;
    return v;
}

void reset_observations(Vision* v) {
    memset(v->observations, -1, sizeof(Observation) * v->n_view_rays);
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

static int intersect_lines(Line line0, Line line1, Vec2* out) {
    Vec2 d0 = line0.b;
    Vec2 d1 = line1.b;

    Vec2 r = sub(line1.position, line0.position);

    float c = d0.x * d1.y - d0.y * d1.x;
    if (fabs(c) < EPS) {
        return 0;
    }

    float t0 = (r.x * d1.y - r.y * d1.x) / c;
    float t1 = (r.x * d0.y - r.y * d0.x) / c;

    if (t0 >= 0 && t0 <= 1 && t1 >= 0 && t1 <= 1) {
        *out = add(line0.position, scale(d0, t0));
        return 1;
    }

    return 0;
}
static int intersect_line_with_circle(
    Line line, Circle circle, Vec2* out
) {
    float x1 = line.position.x;
    float y1 = line.position.y;
    float x2 = x1 + line.b.x;
    float y2 = y1 + line.b.y;
    float cx = circle.position.x;
    float cy = circle.position.y;
    float r = circle.radius;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float a = dx * dx + dy * dy;
    float b = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
    float c = cx * cx + cy * cy + x1 * x1 + y1 * y1
              - 2 * (cx * x1 + cy * y1) - r * r;

    float det = b * b - 4 * a * c;
    int n_points = 0;
    float t1 = 2.0;
    float t2 = 2.0;
    if (det == 0) {
        t1 = -b / (2 * a);
    } else if (det > 0) {
        t1 = (-b + sqrt(det)) / (2 * a);
        t2 = (-b - sqrt(det)) / (2 * a);
    }

    if (t1 >= 0.0 && t1 <= 1.0) {
        out[n_points++] = vec2(x1 + t1 * dx, y1 + t1 * dy);
    }
    if (t2 >= 0.0 && t2 <= 1.0) {
        out[n_points++] = vec2(x1 + t2 * dx, y1 + t2 * dy);
    }

    return n_points;
}

static int intersect_line_with_circle_nearest(
    Line line, Circle circle, Vec2* out
) {
    Vec2 res[2];
    int n_intersects = intersect_line_with_circle(line, circle, res);
    if (n_intersects <= 1) {
        *out = res[0];
        return n_intersects;
    } else if (dist(line.position, res[0]) < dist(line.position, res[1])) {
        *out = res[0];
    } else {
        *out = res[1];
    }
    return 1;
}

static int intersect_line_with_polygon_nearest(
    Line line, Vec2 vertices[], int n, Vec2* out
) {
    float nearest_dist = HUGE_VAL;
    Vec2 nearest_point;

    for (int i = 0; i < n; ++i) {
        Vec2 a = vertices[i];
        Vec2 b = vertices[i < n - 1 ? i + 1 : 0];
        Line side = {a, sub(b, a), 0.0};
        if (intersect_lines(line, side, out)) {
            float curr_dist = dist(line.position, *out);
            if (curr_dist < nearest_dist) {
                nearest_point = *out;
                nearest_dist = curr_dist;
            }
        }
    }

    if (nearest_dist < HUGE_VAL) {
        *out = nearest_point;
        return 1;
    }

    return 0;
}

static int intersect_line_with_primitive_nearest(
    Line line, Primitive primitive, Vec2* out
) {
    switch (primitive.type) {
        case CIRCLE_PRIMITIVE:
            return intersect_line_with_circle_nearest(
                line, primitive.p.circle, out
            );
        default: {
            Vec2 vertices[4];
            int nv = get_primitive_vertices(primitive, vertices);
            return intersect_line_with_polygon_nearest(
                line, vertices, nv, out
            );
        }
    }
}

void observe_world(int entity) {
    if (!entity_has_component(entity, VISION_COMPONENT)) {
        return;
    }

    Vision v = WORLD.vision[entity];
    reset_observations(&v);
    for (int target = 0; target < WORLD.n_entities; ++target) {
        int can_be_observed
            = target != entity
              && entity_has_component(target, COLLIDER_COMPONENT)
              && entity_has_component(target, OBSERVABLE_COMPONENT);
        if (!can_be_observed) {
            continue;
        }

        Primitive collider = WORLD.collider[target];
        int n_view_rays = get_view_rays(v, VIEW_RAYS_ARENA);
        for (int i = 0; i < n_view_rays; ++i) {
            Line ray = {v.position, VIEW_RAYS_ARENA[i], 0.0};
            Observation obs;
            int is_observed = intersect_line_with_primitive_nearest(
                ray, collider, &obs.position
            );
            if (!is_observed) {
                continue;
            }

            float new_dist = dist(v.position, obs.position);
            float old_dist = dist(v.position, v.observations[i].position);
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
                submit_debug_render_command(render_line_command(
                    line(v.position, sub(obs.position, v.position), 0.0),
                    material(GREEN_COLOR)
                ));
                submit_debug_render_command(render_circle_command(
                    circle(obs.position, 0.1, 0.0), material(RED_COLOR)
                ));
            }
        }
    }
}
