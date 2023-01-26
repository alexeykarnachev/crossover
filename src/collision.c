#include "collision.h"

#include "debug/debug.h"
#include "kinematic.h"
#include "math.h"
#include "primitive.h"
#include "renderer.h"
#include "transformation.h"
#include "world.h"
#include <math.h>
#include <stdio.h>

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

static int collide_circles(
    Vec2 p0, float r0, Vec2 p1, float r1, Vec2* mtv
) {
    Vec2 axis = sub(p1, p0);
    float dist = length(axis);
    axis = normalize(axis);
    float radii_sum = r0 + r1;
    if (dist < radii_sum) {
        *mtv = scale(normalize(sub(p1, p0)), dist - radii_sum);
        return 1;
    }
    return 0;
}

static int collide_circle_with_polygon(
    Vec2 position, float radius, Vec2 vertices[], int n, Vec2* mtv
) {
    Vec2 nearest_vertex;
    Vec2 min_overlap_axis;
    float nearest_dist = HUGE_VAL;
    float min_overlap = HUGE_VAL;
    for (int vertex_idx = 0; vertex_idx < n; ++vertex_idx) {
        Vec2 v0 = vertices[vertex_idx];
        Vec2 v1 = vertices[vertex_idx < n - 1 ? vertex_idx + 1 : 0];
        Vec2 axis = normalize(rotate90(sub(v1, v0)));
        Vec2 bound0 = get_circle_proj_bound(position, radius, axis);
        Vec2 bound1 = get_polygon_proj_bound(vertices, n, axis);
        update_overlap(
            bound0, bound1, axis, &min_overlap_axis, &min_overlap
        );

        float curr_dist = dist(v0, position);
        if (curr_dist < nearest_dist) {
            nearest_dist = curr_dist;
            nearest_vertex = v0;
        }
    }

    Vec2 axis = normalize(sub(position, nearest_vertex));
    Vec2 bound0 = get_circle_proj_bound(position, radius, axis);
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

int collide_primitives(
    Primitive p0,
    Transformation t0,
    Primitive p1,
    Transformation t1,
    Collision* collision
) {
    Vec2 v0[4];
    Vec2 v1[4];
    int nv0 = get_primitive_vertices(p0, t0, v0);
    int nv1 = get_primitive_vertices(p1, t1, v1);
    int collided;
    if (p0.type == CIRCLE_PRIMITIVE && p1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circles(
            t0.position,
            p0.p.circle.radius,
            t1.position,
            p1.p.circle.radius,
            &collision->mtv
        );
    } else if (p0.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            t0.position, p0.p.circle.radius, v1, nv1, &collision->mtv
        );
        collision->mtv = flip(collision->mtv);
    } else if (p1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            t1.position, p1.p.circle.radius, v0, nv0, &collision->mtv
        );
    } else {
        collided = collide_polygons(v0, nv0, v1, nv1, &collision->mtv);
    }

    return collided;
}

void compute_collisions() {
    WORLD.n_collisions = 0;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_can_collide(entity)) {
            continue;
        }

        Primitive p0 = WORLD.collider[entity];
        Transformation t0 = WORLD.transformation[entity];

        for (int target = entity + 1; target < WORLD.n_entities;
             ++target) {
            if (!entity_can_collide(target)) {
                continue;
            }

            Primitive p1 = WORLD.collider[target];
            Transformation t1 = WORLD.transformation[target];
            Collision* c = &WORLD.collisions[WORLD.n_collisions];
            c->entity0 = entity;
            c->entity1 = target;

            int collided = collide_primitives(p0, t0, p1, t1, c);
            WORLD.n_collisions += collided;
            if (DEBUG.shading.collisions) {
                if (collided) {
                    render_debug_line(
                        t0.position,
                        add(t0.position, c->mtv),
                        MAGENTA_COLOR
                    );
                    render_debug_line(
                        t1.position, sub(t1.position, c->mtv), CYAN_COLOR
                    );
                }
            }
        }

        if (DEBUG.shading.collisions) {
            render_debug_primitive(t0, p0, SKYBLUE_COLOR, LINE);
        }
    }
}

void resolve_collisions() {
    if (DEBUG.collisions.resolve || DEBUG.collisions.resolve_once) {
        DEBUG.collisions.resolve_once = 0;

        for (int i = 0; i < WORLD.n_collisions; ++i) {
            Collision collision = WORLD.collisions[i];

            Vec2 mtv = collision.mtv;
            int e0 = collision.entity0;
            int e1 = collision.entity1;

            Transformation* t0 = &WORLD.transformation[e0];
            Transformation* t1 = &WORLD.transformation[e1];
            if (entity_has_rigid_body(e0) && entity_has_rigid_body(e1)) {
                int has_kinematic0 = entity_has_kinematic(e0);
                int has_kinematic1 = entity_has_kinematic(e1);
                if (has_kinematic0 && has_kinematic1) {
                    t0->position = add(t0->position, scale(mtv, 0.5));
                    t1->position = add(t1->position, scale(mtv, -0.5));
                } else if (has_kinematic0) {
                    t0->position = add(t0->position, mtv);
                } else if (has_kinematic1) {
                    t1->position = add(t1->position, flip(mtv));
                }
            }

            if (entity_can_be_damaged_by_bullet(e1, e0)) {
                WORLD.health[e0] -= get_kinematic_damage(
                    WORLD.kinematic[e1]
                );
            } else if (entity_can_be_damaged_by_bullet(e0, e1)) {
                WORLD.health[e1] -= get_kinematic_damage(
                    WORLD.kinematic[e0]
                );
            }

            if (bullet_can_be_destroyed_after_collision(e1, e0)) {
                destroy_entity(e1);
            } else if (bullet_can_be_destroyed_after_collision(e0, e1)) {
                destroy_entity(e0);
            }
        }
    }
}
