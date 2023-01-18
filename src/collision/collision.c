#include "collision.h"

#include "../debug/debug.h"
#include "../movement.h"
#include "../primitive.h"
#include "../world.h"
#include "math.h"
#include <math.h>
#include <stdio.h>

static Vec2 get_circle_proj_bound(Circle c, Vec2 axis) {
    axis = normalize(axis);
    Vec2 r = scale(axis, c.radius);
    float k0 = dot(add(c.position, r), axis);
    float k1 = dot(add(c.position, scale(r, -1.0)), axis);

    return k1 > k0 ? vec2(k0, k1) : vec2(k1, k0);
}

static Vec2 get_polygon_proj_bound(Vec2 points[], int n, Vec2 axis) {
    float min_k = HUGE_VAL;
    float max_k = -HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        float k = dot(points[i], axis);
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
    axis = bound0.x < bound1.x ? scale(axis, -1.0) : axis;
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

static int collide_circles(Circle c0, Circle c1, Vec2* mtv) {
    float dist = dist_between_points(c0.position, c1.position);
    float radii_sum = c0.radius + c1.radius;
    if (dist < radii_sum) {
        *mtv = scale(
            normalize(sub(c1.position, c0.position)), dist - radii_sum
        );
        return 1;
    }
    return 0;
}

static Primitive project_circle_on_axis(
    Circle circle, Vec2 axis, float offset
) {
    axis = normalize(axis);
    Vec2 radius = scale(axis, circle.radius);
    float k0 = dot(axis, add(circle.position, radius));
    float k1 = dot(axis, add(circle.position, scale(radius, -1.0)));
    Vec2 position = scale(axis, k0);
    Vec2 b = sub(scale(axis, k1), position);

    Vec2 normal = rotate90(axis);
    position = add(position, scale(normal, offset));

    return line_primitive(position, b);
}

static Primitive project_polygon_on_axis(
    Vec2 vertices[], int n, Vec2 axis, float offset
) {
    axis = normalize(axis);
    float k0 = HUGE_VAL;
    float k1 = -HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        float k = dot(axis, vertices[i]);
        k0 = min(k0, k);
        k1 = max(k1, k);
    }
    Vec2 position = scale(axis, k0);
    Vec2 b = sub(scale(axis, k1), position);

    Vec2 normal = rotate90(axis);
    position = add(position, scale(normal, offset));

    return line_primitive(position, b);
}

static void render_collision_axis(
    Vec2 axis, Circle circle, Vec2 vertices[], int n
) {
    if (!DEBUG.shading.collision_axis) {
        return;
    }

    Primitive circle_proj = project_circle_on_axis(circle, axis, 0.05);
    Primitive polygon_proj = project_polygon_on_axis(
        vertices, n, axis, -0.05
    );
    submit_debug_render_command(render_primitive_command(
        circle_proj, material(vec3(1.0, 0.0, 1.0))
    ));
    submit_debug_render_command(render_primitive_command(
        polygon_proj, material(vec3(1.0, 1.0, 0.0))
    ));
}

static void render_mtv(
    Vec2 mtv, Vec2 nearest_vertex, Vec2 circle_position
) {
    if (!DEBUG.shading.mtv) {
        return;
    }

    Primitive line0 = line_primitive(circle_position, mtv);
    Primitive line1 = line_primitive(nearest_vertex, scale(mtv, -1.0));
    submit_debug_render_command(
        render_primitive_command(line0, default_mtv_material())
    );
    submit_debug_render_command(
        render_primitive_command(line1, default_mtv_material())
    );
}

static int collide_circle_with_polygon(
    Circle circle, Vec2 vertices[], int n, Vec2* mtv
) {
    Vec2 nearest_vertex;
    float nearest_dist = HUGE_VAL;
    for (int vertex_idx = 0; vertex_idx < n; ++vertex_idx) {
        Vec2 v = vertices[vertex_idx];
        float dist = dist_between_points(v, circle.position);
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_vertex = v;
        }
    }

    Vec2 min_overlap_axis;
    float min_overlap = HUGE_VAL;
    for (int vertex_idx = 0; vertex_idx < n; ++vertex_idx) {
        Vec2 v0 = vertices[vertex_idx];
        Vec2 v1 = vertices[vertex_idx < n - 1 ? vertex_idx + 1 : 0];
        Vec2 axis = normalize(rotate90(sub(v1, v0)));
        Vec2 bound0 = get_circle_proj_bound(circle, axis);
        Vec2 bound1 = get_polygon_proj_bound(vertices, n, axis);
        update_overlap(
            bound0, bound1, axis, &min_overlap_axis, &min_overlap
        );
    }

    Vec2 axis = normalize(sub(circle.position, nearest_vertex));
    Vec2 bound0 = get_circle_proj_bound(circle, axis);
    Vec2 bound1 = get_polygon_proj_bound(vertices, n, axis);
    update_overlap(bound0, bound1, axis, &min_overlap_axis, &min_overlap);

    if (min_overlap > 0) {
        *mtv = scale(min_overlap_axis, min_overlap);
        render_collision_axis(min_overlap_axis, circle, vertices, n);
        render_mtv(*mtv, nearest_vertex, circle.position);
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

    return collided;
}

void resolve_collision(Collision collision) {
    Vec2 mtv = collision.mtv;
    int e0 = collision.entity0;
    int e1 = collision.entity1;
    int has_movement0 = entity_has_component(e0, MOVEMENT_COMPONENT);
    int has_movement1 = entity_has_component(e1, MOVEMENT_COMPONENT);
    if (has_movement0 && has_movement1) {
        translate_entity(e0, scale(mtv, 0.5));
        translate_entity(e1, scale(mtv, -0.5));
    } else if (has_movement0) {
        translate_entity(e0, mtv);
    } else if (has_movement1) {
        translate_entity(e0, scale(mtv, -1.0));
    }
}
