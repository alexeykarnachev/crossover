#include "collision.h"

#include "../debug/debug.h"
#include "../movement.h"
#include "../primitive.h"
#include "../world.h"
#include "math.h"
#include <math.h>
#include <stdio.h>

static Vec2 get_circle_proj_bound(Circle c, Vec2 axis) {
    Vec2 r = scale_vec2(normalize_vec2(axis), c.radius);
    Vec2 p0 = normalize_vec2(add_vec2(c.position, r));
    Vec2 p1 = normalize_vec2(add_vec2(c.position, scale_vec2(r, -1.0)));
    float k0 = dot_vec2(p0, axis);
    float k1 = dot_vec2(p1, axis);
    return k1 > k0 ? vec2(k0, k1) : vec2(k1, k0);
}

static Vec2 get_polygon_proj_bound(Vec2 points[], int n, Vec2 axis) {
    float min_k = HUGE_VAL;
    float max_k = -HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        float k = dot_vec2(normalize_vec2(points[i]), axis);
        min_k = min(min_k, k);
        max_k = max(max_k, k);
    }
    return vec2(min_k, max_k);
}

static void update_overlap(
    Vec2 bound0,
    Vec2 bound1,
    Vec2 axis,
    Vec2* min_overlap_axis,
    float* min_overlap
) {
    float r0 = 0.5 * (bound0.y - bound0.x);
    float r1 = 0.5 * (bound1.y - bound1.x);
    float c0 = 0.5 * (bound0.y + bound0.x);
    float c1 = 0.5 * (bound1.y + bound1.x);
    float radii_sum = r0 + r1;
    float dist = fabs(c1 - c0);
    float overlap = radii_sum - dist;
    if (overlap < *min_overlap) {
        *min_overlap = overlap;
        *min_overlap_axis = axis;
    }
}

static void update_circle_polygon_overlap(
    Circle circle,
    Vec2 vertices[],
    int n,
    Vec2 axis,
    Vec2* min_overlap_axis,
    float* min_overlap
) {
    Vec2 bound0 = get_circle_proj_bound(circle, axis);
    Vec2 bound1 = get_polygon_proj_bound(vertices, n, axis);
    update_overlap(bound0, bound1, axis, min_overlap_axis, min_overlap);
}

static int collide_circles(Circle c0, Circle c1, Vec2* mtv) {
    float dist = dist_between_points(c0.position, c1.position);
    float radii_sum = c0.radius + c1.radius;
    if (dist < radii_sum) {
        *mtv = scale_vec2(
            normalize_vec2(sub_vec2(c1.position, c0.position)),
            dist - radii_sum
        );
        return 1;
    }
    return 0;
}

static int collide_circle_with_polygon(
    Circle circle, Vec2 vertices[], int n, Vec2* mtv
) {
    Vec2 min_overlap_axis;
    Vec2 nearest_vertex;
    float min_overlap = HUGE_VAL;
    float nearest_dist = HUGE_VAL;
    for (int vertex_idx = 0; vertex_idx < n; ++vertex_idx) {
        Vec2 v0 = vertices[vertex_idx];
        Vec2 v1 = vertices[vertex_idx < n - 1 ? vertex_idx + 1 : 0];
        Vec2 axis = rotate90(sub_vec2(v1, v0));
        if (DEBUG.shading.collision_axis) {
            // render_debug_primitive()
        }

        axis = normalize_vec2(axis);

        update_circle_polygon_overlap(
            circle, vertices, n, axis, &min_overlap_axis, &min_overlap
        );

        if (min_overlap <= 0) {
            return 0;
        }

        float dist = dist_between_points(v0, circle.position);
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_vertex = v0;
        }
    }

    Vec2 axis = normalize_vec2(sub_vec2(circle.position, nearest_vertex));
    update_circle_polygon_overlap(
        circle, vertices, n, axis, &min_overlap_axis, &min_overlap
    );

    if (min_overlap > 0) {
        *mtv = scale_vec2(normalize_vec2(axis), 2.0);
        return 1;
    }

    return 0;
}

int collide_entities(int e0, int e1, Collision* collision) {

    int can_collide = entity_has_component(e0, COLLIDER_COMPONENT)
                      && entity_has_component(e1, COLLIDER_COMPONENT);
    if (!can_collide) {
        return 0;
    }

    Vec2 v0[4];
    Vec2 v1[4];
    collision->entity0 = e0;
    collision->entity1 = e1;
    Primitive p0 = WORLD.primitive[e0];
    Primitive p1 = WORLD.primitive[e1];
    int nv0 = get_primitive_vertices(p0, v0);
    int nv1 = get_primitive_vertices(p1, v1);
    int collided;
    if (p0.type == CIRCLE_PRIMITIVE && p1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circles(
            p0.p.circle, p1.p.circle, &collision->mtv
        );
    } else if (p0.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            p0.p.circle, v1, nv1, &collision->mtv
        );
    } else if (p1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            p1.p.circle, v0, nv0, &collision->mtv
        );
    } else {
        return 0;
    }

    if (collided && DEBUG.shading.mtv) {
        Vec2 pos0 = get_primitive_position(p0);
        Vec2 pos1 = get_primitive_position(p1);
        Primitive line0 = line_primitive(pos0, collision->mtv);
        Primitive line1 = line_primitive(
            pos1, scale_vec2(collision->mtv, -1.0)
        );
        submit_debug_render_command(
            render_primitive_command(line0, default_mtv_material())
        );
        submit_debug_render_command(
            render_primitive_command(line1, default_mtv_material())
        );
    }

    return collided;
}

void resolve_collision(Collision collision) {
    Vec2 mtv = collision.mtv;
    int e0 = collision.entity0;
    int e1 = collision.entity1;
    int has_movement0 = entity_has_component(e0, MOVEMENT_COMPONENT);
    int has_movement1 = entity_has_component(e1, MOVEMENT_COMPONENT);
    if (has_movement0 && has_movement1) {
        translate_entity(e0, scale_vec2(mtv, 0.5));
        translate_entity(e1, scale_vec2(mtv, -0.5));
    } else if (has_movement0) {
        translate_entity(e0, mtv);
    } else if (has_movement1) {
        translate_entity(e0, scale_vec2(mtv, -1.0));
    }
}
