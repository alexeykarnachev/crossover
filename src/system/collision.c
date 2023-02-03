#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static CollisionsArena COLLISIONS_ARENA;

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
    Primitive primitive0,
    Transformation transformation0,
    Primitive primitive1,
    Transformation transformation1,
    Collision* collision
) {
    Vec2 vertices0[MAX_N_POLYGON_VERTICES];
    Vec2 vertices1[MAX_N_POLYGON_VERTICES];
    int nv0 = get_primitive_vertices(primitive0, vertices0);
    int nv1 = get_primitive_vertices(primitive1, vertices1);
    apply_transformation(vertices0, nv0, transformation0);
    apply_transformation(vertices1, nv1, transformation1);
    int collided;
    if (primitive0.type == CIRCLE_PRIMITIVE
        && primitive1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circles(
            transformation0.position,
            primitive0.p.circle.radius,
            transformation1.position,
            primitive1.p.circle.radius,
            &collision->mtv
        );
    } else if (primitive0.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            transformation0.position,
            primitive0.p.circle.radius,
            vertices1,
            nv1,
            &collision->mtv
        );
        collision->mtv = flip(collision->mtv);
    } else if (primitive1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            transformation1.position,
            primitive1.p.circle.radius,
            vertices0,
            nv0,
            &collision->mtv
        );
    } else {
        collided = collide_polygons(
            vertices0, nv0, vertices1, nv1, &collision->mtv
        );
        collision->mtv = flip(collision->mtv);
    }

    return collided;
}

static void compute_collisions() {
    COLLISIONS_ARENA.n = 0;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, CAN_COLLIDE_COMPONENT)) {
            continue;
        }

        Primitive primitive0 = WORLD.colliders[entity];
        Transformation transformation0 = WORLD.transformations[entity];

        for (int target = entity + 1; target < WORLD.n_entities;
             ++target) {
            if (!entity_has_component(target, CAN_COLLIDE_COMPONENT)) {
                continue;
            }

            Primitive primitive1 = WORLD.colliders[target];
            Transformation transformation1 = WORLD.transformations[target];
            Collision* collision
                = &COLLISIONS_ARENA.arena[COLLISIONS_ARENA.n];
            collision->entity0 = entity;
            collision->entity1 = target;

            int collided = collide_primitives(
                primitive0,
                transformation0,
                primitive1,
                transformation1,
                collision
            );
            COLLISIONS_ARENA.n += collided;
        }
    }

    DEBUG.general.n_collisions = COLLISIONS_ARENA.n;
}

static void resolve_collisions() {
    if (DEBUG.collisions.resolve || DEBUG.collisions.resolve_once) {
        DEBUG.collisions.resolve_once = 0;

        for (int i = 0; i < COLLISIONS_ARENA.n; ++i) {
            Collision collision = COLLISIONS_ARENA.arena[i];

            Vec2 mtv = collision.mtv;
            int entity0 = collision.entity0;
            int entity1 = collision.entity1;

            Transformation* transformation0
                = &WORLD.transformations[entity0];
            Transformation* transformation1
                = &WORLD.transformations[entity1];
            if (entity_has_component(entity0, RIGID_BODY_COMPONENT)
                && entity_has_component(entity1, RIGID_BODY_COMPONENT)) {
                int has_kinematic0 = entity_has_component(
                    entity0, KINEMATIC_COMPONENT
                );
                int has_kinematic1 = entity_has_component(
                    entity1, KINEMATIC_COMPONENT
                );
                if (has_kinematic0 && has_kinematic1) {
                    transformation0->position = add(
                        transformation0->position, scale(mtv, 0.5)
                    );
                    transformation1->position = add(
                        transformation1->position, scale(mtv, -0.5)
                    );
                } else if (has_kinematic0) {
                    transformation0->position = add(
                        transformation0->position, mtv
                    );
                } else if (has_kinematic1) {
                    transformation1->position = add(
                        transformation1->position, flip(mtv)
                    );
                }
            }
        }
    }
}

void update_collisions() {
    compute_collisions();
    resolve_collisions();
}

void render_debug_collisions() {
    for (int i = 0; i < COLLISIONS_ARENA.n; ++i) {
        Collision collision = COLLISIONS_ARENA.arena[i];
        Transformation transformation0
            = WORLD.transformations[collision.entity0];
        Transformation transformation1
            = WORLD.transformations[collision.entity1];

        render_debug_line(
            transformation0.position,
            add(transformation0.position, collision.mtv),
            MAGENTA_COLOR,
            DEBUG_RENDER_LAYER
        );
        render_debug_line(
            transformation1.position,
            sub(transformation1.position, collision.mtv),
            CYAN_COLOR,
            DEBUG_RENDER_LAYER
        );
    }

    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, CAN_COLLIDE_COMPONENT)) {
            continue;
        }

        Transformation transformation = WORLD.transformations[entity];
        Primitive primitive = WORLD.colliders[entity];
        render_debug_primitive(
            transformation,
            primitive,
            SKYBLUE_COLOR,
            DEBUG_RENDER_LAYER,
            LINE
        );
    }
}
