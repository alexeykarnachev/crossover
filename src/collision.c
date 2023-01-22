#include "collision.h"

#include "debug/debug.h"
#include "math.h"
#include "movement.h"
#include "primitive.h"
#include "world.h"
#include <math.h>
#include <stdio.h>

Line project_circle_on_axis(Circle circle, Vec2 axis) {
    axis = normalize(axis);
    Vec2 radius = scale(axis, circle.radius);
    float k0 = dot(axis, add(circle.position, radius));
    float k1 = dot(axis, add(circle.position, scale(radius, -1.0)));
    Vec2 position = scale(axis, k0);
    Vec2 b = sub(scale(axis, k1), position);

    return line(position, b, 0.0);
}

Line project_polygon_on_axis(Vec2 vertices[], int n, Vec2 axis) {
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

    return line(position, b, 0.0);
}

static Vec2 get_circle_proj_bound(Circle c, Vec2 axis) {
    axis = normalize(axis);
    Vec2 r = scale(axis, c.radius);
    float k0 = dot(sub(c.position, r), axis);
    float k1 = dot(add(c.position, r), axis);
    return vec2(k0, k1);
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
    float r0 = 0.5 * (bound0.y - bound0.x);
    float r1 = 0.5 * (bound1.y - bound1.x);
    float c0 = 0.5 * (bound0.y + bound0.x);
    float c1 = 0.5 * (bound1.y + bound1.x);
    float radii_sum = r0 + r1;
    float dist = fabs(c1 - c0);
    float overlap = radii_sum - dist;
    if (overlap < *min_overlap) {
        *min_overlap = overlap;
        *min_overlap_axis = c1 - c0 < 0 ? flip(axis) : axis;
    }
}

static void render_mtv(Vec2 mtv, Vec2 position0, Vec2 position1) {
    Line line0 = line(position0, mtv, 0.0);
    Line line1 = line(position1, scale(mtv, -1.0), 0.0);
    submit_debug_render_command(
        render_line_command(line0, material(MAGENTA_COLOR))
    );
    submit_debug_render_command(
        render_line_command(line1, material(CYAN_COLOR))
    );
}

static void render_circle_polygon_proj(
    Vec2 axis, Circle circle, Vec2 vertices[], int n
) {
    axis = normalize(axis);

    Vec2 origin = add(circle.position, add_many(vertices, n));
    origin = scale(origin, 1.0 / (n + 1));

    Vec2 offset = point_to_axis(origin, axis);
    Vec2 offset0 = add(offset, scale(normalize(offset), 1.0));
    Vec2 offset1 = add(offset, scale(normalize(offset), 1.1));

    Line proj0 = project_circle_on_axis(circle, axis);
    Line proj1 = project_polygon_on_axis(vertices, n, axis);
    proj0.position = sub(proj0.position, offset0);
    proj1.position = sub(proj1.position, offset1);

    submit_debug_render_command(
        render_line_command(proj0, material(MAGENTA_COLOR))
    );
    submit_debug_render_command(
        render_line_command(proj1, material(CYAN_COLOR))
    );
}

static void render_circles_proj(
    Vec2 axis, Circle circle0, Circle circle1
) {
    axis = normalize(axis);

    Vec2 origin = add(circle0.position, circle1.position);
    origin = scale(origin, 0.5);

    Vec2 offset = point_to_axis(origin, axis);
    Vec2 offset0 = add(offset, scale(normalize(offset), 1.0));
    Vec2 offset1 = add(offset, scale(normalize(offset), 1.1));

    Line proj0 = project_circle_on_axis(circle0, axis);
    Line proj1 = project_circle_on_axis(circle1, axis);
    proj0.position = sub(proj0.position, offset0);
    proj1.position = sub(proj1.position, offset1);

    submit_debug_render_command(
        render_line_command(proj0, material(MAGENTA_COLOR))
    );
    submit_debug_render_command(
        render_line_command(proj1, material(CYAN_COLOR))
    );
}

static void render_polygons_proj(
    Vec2 axis, Vec2 vertices0[], int n0, Vec2 vertices1[], int n1
) {
    axis = normalize(axis);

    Vec2 origin = add(add_many(vertices0, n0), add_many(vertices1, n1));
    origin = scale(origin, 1.0 / (n0 + n1));

    Vec2 offset = point_to_axis(origin, axis);
    Vec2 offset0 = add(offset, scale(normalize(offset), 1.0));
    Vec2 offset1 = add(offset, scale(normalize(offset), 1.1));

    Line proj0 = project_polygon_on_axis(vertices0, n0, axis);
    Line proj1 = project_polygon_on_axis(vertices1, n1, axis);
    proj0.position = sub(proj0.position, offset0);
    proj1.position = sub(proj1.position, offset1);

    submit_debug_render_command(
        render_line_command(proj0, material(MAGENTA_COLOR))
    );
    submit_debug_render_command(
        render_line_command(proj1, material(CYAN_COLOR))
    );
}

static int collide_circles(Circle c0, Circle c1, Vec2* mtv) {
    Vec2 axis = sub(c1.position, c0.position);
    float dist = length(axis);
    axis = normalize(axis);
    float radii_sum = c0.radius + c1.radius;
    if (dist < radii_sum) {
        *mtv = scale(
            normalize(sub(c1.position, c0.position)), dist - radii_sum
        );
        return 1;
    }
    return 0;
}

static int collide_circle_with_polygon(
    Circle circle, Vec2 vertices[], int n, Vec2* mtv
) {
    Vec2 nearest_vertex;
    Vec2 min_overlap_axis;
    float nearest_dist = HUGE_VAL;
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

        float curr_dist = dist(v0, circle.position);
        if (curr_dist < nearest_dist) {
            nearest_dist = curr_dist;
            nearest_vertex = v0;
        }
    }

    Vec2 axis = normalize(sub(circle.position, nearest_vertex));
    Vec2 bound0 = get_circle_proj_bound(circle, axis);
    Vec2 bound1 = get_polygon_proj_bound(vertices, n, axis);
    update_overlap(bound0, bound1, axis, &min_overlap_axis, &min_overlap);

    if (min_overlap > 0) {
        *mtv = scale(min_overlap_axis, min_overlap);
        return 1;
    }

    return 0;
}

static int collide_polygons(
    Vec2 vertices0[], int n0, Vec2 vertices1[], int n1, Vec2* mtv
) {
    Vec2 min_overlap_axis;
    float min_overlap = HUGE_VAL;
    for (int i = 0; i < n0 + n1; i++) {
        Vec2* vertices;
        int n;
        if (i < n0) {
            vertices = vertices0;
            n = n0;
        } else {
            vertices = vertices1;
            n = n1;
        }
        for (int vertex_idx = 0; vertex_idx < n; ++vertex_idx) {
            Vec2 v0 = vertices[vertex_idx];
            Vec2 v1 = vertices[vertex_idx < n - 1 ? vertex_idx + 1 : 0];
            Vec2 axis = normalize(rotate90(sub(v1, v0)));
            Vec2 bound0 = get_polygon_proj_bound(vertices0, n0, axis);
            Vec2 bound1 = get_polygon_proj_bound(vertices1, n1, axis);
            update_overlap(
                bound0, bound1, axis, &min_overlap_axis, &min_overlap
            );
        }
    }
    if (min_overlap > 0) {
        *mtv = scale(min_overlap_axis, min_overlap);
        return 1;
    }
    return 0;
}

int collide_primitives(Primitive p0, Primitive p1, Collision* collision) {
    Vec2 v0[4];
    Vec2 v1[4];
    int nv0 = get_primitive_vertices(p0, v0);
    int nv1 = get_primitive_vertices(p1, v1);
    int collided;
    if (p0.type == CIRCLE_PRIMITIVE && p1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circles(
            p0.p.circle, p1.p.circle, &collision->mtv
        );
        if (collided && DEBUG.shading.collision) {
            render_circles_proj(collision->mtv, p0.p.circle, p1.p.circle);
            render_mtv(
                collision->mtv, p0.p.circle.position, p1.p.circle.position
            );
        }
    } else if (p0.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            p0.p.circle, v1, nv1, &collision->mtv
        );
        collision->mtv = flip(collision->mtv);
        if (collided && DEBUG.shading.collision && nv1 > 2) {
            render_circle_polygon_proj(
                collision->mtv, p0.p.circle, v1, nv1
            );
            render_mtv(collision->mtv, p0.p.circle.position, v1[0]);
        }
    } else if (p1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            p1.p.circle, v0, nv0, &collision->mtv
        );
        if (collided && DEBUG.shading.collision && nv0 > 2) {
            render_circle_polygon_proj(
                collision->mtv, p1.p.circle, v0, nv0
            );
            render_mtv(collision->mtv, v0[0], p1.p.circle.position);
        }
    } else {
        collided = collide_polygons(v0, nv0, v1, nv1, &collision->mtv);
        if (collided && DEBUG.shading.collision && min(nv0, nv1) > 2) {
            render_polygons_proj(collision->mtv, v0, nv0, v1, nv1);
            render_mtv(collision->mtv, v0[0], v1[0]);
        }
    }

    return collided;
}

int collide_entities(int e0, int e1, Collision* collision) {
    int can_collide = entity_has_component(e0, COLLIDER_COMPONENT)
                      && entity_has_component(e1, COLLIDER_COMPONENT);
    if (!can_collide) {
        return 0;
    }

    collision->entity0 = e0;
    collision->entity1 = e1;
    Primitive p0 = WORLD.collider[e0];
    Primitive p1 = WORLD.collider[e1];

    return collide_primitives(p0, p1, collision);
}

void resolve_collision(Collision collision) {
    Vec2 mtv = collision.mtv;
    int e0 = collision.entity0;
    int e1 = collision.entity1;
    int has_movement0 = entity_has_component(e0, MOVEMENT_COMPONENT);
    int has_movement1 = entity_has_component(e1, MOVEMENT_COMPONENT);
    if (has_movement0 && has_movement1) {
        transform_entity(e0, scale(mtv, 0.5), 0.0);
        transform_entity(e1, scale(mtv, -0.5), 0.0);
    } else if (has_movement0) {
        transform_entity(e0, mtv, 0.0);
    } else if (has_movement1) {
        transform_entity(e1, flip(mtv), 0.0);
    }
}
