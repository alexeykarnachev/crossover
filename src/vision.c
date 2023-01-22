#include "vision.h"

#include "debug/debug.h"
#include "math.h"
#include "renderer.h"
#include "world.h"
#include <math.h>
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

static int intersect_lines(Line line0, Line line1, Vec2* out) {
    Vec2 d0 = line0.b;
    Vec2 d1 = line1.b;

    Vec2 s0 = line0.position;
    Vec2 e0 = add(s0, d0);
    Vec2 s1 = line1.position;
    Vec2 e1 = add(s1, d1);

    Vec2 r = sub(s0, s1);

    float c0 = d0.x * d1.y - d0.y * d1.x;
    if (fabs(c0) < EPS) {
        return 0;
    }

    float c1 = r.x * d1.y - r.y * d1.x;
    float t = -c1 / c0;
    Vec2 p = add(s0, scale(d0, t));

    int on_line0 = p.x >= min(s0.x, e0.x) && p.x <= max(s0.x, e0.x)
                   && p.y >= min(s0.y, e0.y) && p.y <= max(s0.y, e0.y);
    int on_line1 = p.x >= min(s1.x, e1.x) && p.x <= max(s1.x, e1.x)
                   && p.y >= min(s1.y, e1.y) && p.y <= max(s1.y, e1.y);
    if (on_line0 && on_line1) {
        *out = p;
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
    if (det < 0) {
        return 0;
    } else if (det == 0) {
        // one intersection point
        float t = -b / (2 * a);
        if (t >= 0.0 && t <= 1.0) {
            out[0].x = x1 + t * dx;
            out[0].y = y1 + t * dy;
            return 1;
        }
    } else {
        // two intersection points
        float t1 = (-b + sqrt(det)) / (2 * a);
        float t2 = (-b - sqrt(det)) / (2 * a);
        if (t1 >= 0.0 && t1 <= 1.0 && t2 >= 0.0 && t2 <= 1.0) {
            out[0].x = x1 + t1 * dx;
            out[0].y = y1 + t1 * dy;
            out[1].x = x1 + t2 * dx;
            out[1].y = y1 + t2 * dy;
            return 2;
        } else if (t1 >= 0.0 && t1 <= 1.0) {
            out[0].x = x1 + t1 * dx;
            out[0].y = y1 + t1 * dy;
            return 1;
        } else if (t2 >= 0.0 && t2 <= 1.0) {
            out[0].x = x1 + t2 * dx;
            out[0].y = y1 + t2 * dy;
            return 1;
        } else {
            return 0;
        }
    }
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
        Line ray = {v.position, VIEW_RAYS_ARENA[i], 0.0};
        Vec2 point;
        if (intersect_line_with_primitive_nearest(ray, collider, &point)) {
            submit_debug_render_command(
                render_line_command(ray, material(GREEN_COLOR))
            );
            submit_debug_render_command(render_circle_command(
                circle(point, 0.1, 0.0), material(RED_COLOR)
            ));
        }
    }
}
