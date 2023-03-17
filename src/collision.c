#include "collision.h"

#include "component.h"
#include "const.h"
#include "debug.h"
#include "editor.h"
#include "gl.h"
#include "math.h"
#include "profiler.h"
#include "scene.h"
#include "system.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Collision COLLISIONS[MAX_N_COLLISIONS];
int N_COLLISIONS = 0;

void push_collision(Collision collision) {
    if (N_COLLISIONS == MAX_N_COLLISIONS) {
        fprintf(
            stderr,
            "ERROR: Engine can't process more than %d collisions\n",
            MAX_N_COLLISIONS
        );
        exit(1);
    }

    COLLISIONS[N_COLLISIONS++] = collision;
}

Collision pop_collision(void) {
    if (N_COLLISIONS == 0) {
        fprintf(
            stderr,
            "ERROR: Can't pop collision from the empty collisions array\n"
        );
        exit(1);
    }
    Collision collision = COLLISIONS[--N_COLLISIONS];
    return collision;
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

static int collide_circles(
    Vec2 p0, float r0, Vec2 p1, float r1, Collision* collision
) {
    Vec2 axis = sub(p1, p0);
    float dist = length(axis);
    axis = normalize(axis);
    float radii_sum = r0 + r1;
    if (dist < radii_sum) {
        Vec2 dp = sub(p1, p0);
        Vec2 dir = length(dp) > EPS ? normalize(dp) : vec2(1.0, 0.0);
        collision->mtv = scale(dir, dist - radii_sum);
        return 1;
    }
    return 0;
}

static int collide_circle_with_polygon(
    Vec2 position,
    float radius,
    Vec2 vertices[],
    int n,
    Collision* collision
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
        collision->mtv = scale(min_overlap_axis, min_overlap);
        return 1;
    }

    return 0;
}

static int collide_polygons(
    Vec2 vertices0[],
    int n0,
    Vec2 vertices1[],
    int n1,
    Collision* collision
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
        collision->mtv = scale(min_overlap_axis, min_overlap);
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
            transformation0.curr_position,
            primitive0.p.circle.radius,
            transformation1.curr_position,
            primitive1.p.circle.radius,
            collision
        );
    } else if (primitive0.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            transformation0.curr_position,
            primitive0.p.circle.radius,
            vertices1,
            nv1,
            collision
        );
        collision->mtv = flip(collision->mtv);
    } else if (primitive1.type == CIRCLE_PRIMITIVE) {
        collided = collide_circle_with_polygon(
            transformation1.curr_position,
            primitive1.p.circle.radius,
            vertices0,
            nv0,
            collision
        );
    } else {
        collided = collide_polygons(
            vertices0, nv0, vertices1, nv1, collision
        );
        collision->mtv = flip(collision->mtv);
    }

    return collided;
}

static void update_tiling(void) {
    int required_component = TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }
        Transformation transformation = SCENE.transformations[entity];
        int* need_update_tiling = &SCENE.need_update_tiling[entity];

        // NOTE: Don't update tiling if the entity didn't change the
        // position
        if (*need_update_tiling == 0) {
            continue;
        }
        *need_update_tiling = 0;

        entity_leaves_all_tiles(entity);
        Primitive collider = SCENE.colliders[entity];

        float right_x = -FLT_MAX;
        float left_x = FLT_MAX;
        float top_y = -FLT_MAX;
        float bot_y = FLT_MAX;
        if (collider.type == CIRCLE_PRIMITIVE) {
            float x = transformation.curr_position.x;
            float y = transformation.curr_position.y;
            float r = collider.p.circle.radius;
            right_x = x + r;
            left_x = x - r;
            top_y = y + r;
            bot_y = y - r;
        } else {
            static Vec2 vertices[MAX_N_POLYGON_VERTICES];
            int n_vertices = get_primitive_vertices(collider, vertices);
            apply_transformation(vertices, n_vertices, transformation);

            for (int i = 0; i < n_vertices; ++i) {
                Vec2 vert = vertices[i];
                right_x = max(right_x, vert.x);
                left_x = min(left_x, vert.x);
                top_y = max(top_y, vert.y);
                bot_y = min(bot_y, vert.y);
            }
        }

        Vec2 start = get_tile_location_at(vec2(left_x, top_y));
        Vec2 last = get_tile_location_at(vec2(right_x, bot_y));
        Vec2 curr = start;
        do {
            int tile = curr.y * N_X_SCENE_TILES + curr.x;
            if (curr.x != -1 && curr.y != -1 && tile >= 0
                && tile < N_SCENE_TILES) {
                entity_enters_tile(entity, tile);
            }

            if (curr.x == last.x) {
                curr.x = start.x;
                curr.y += 1;
            } else {
                curr.x += 1;
            }
        } while (curr.x <= last.x && curr.y <= last.y);
    }
}

static int check_if_entities_can_collide(int entity0, int entity1) {
    int has_rb0 = check_if_entity_has_component(
        entity0, RIGID_BODY_COMPONENT
    );
    int has_rb1 = check_if_entity_has_component(
        entity1, RIGID_BODY_COMPONENT
    );

    if (has_rb0 == 0 || has_rb1 == 0) {
        return 0;
    }

    RigidBody* rb0 = &SCENE.rigid_bodies[entity0];
    RigidBody* rb1 = &SCENE.rigid_bodies[entity1];
    if (rb0->type != DYNAMIC_RIGID_BODY
        && rb1->type != DYNAMIC_RIGID_BODY) {
        return 0;
    }

    return 1;
}

static void compute_collisions(void) {
    if (N_COLLISIONS != 0) {
        fprintf(
            stderr,
            "ERROR: Can't compute new collisions while the current "
            "collisions array is not resolved\n"
        );
        exit(1);
    }

    int required_component = TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        Primitive primitive0 = SCENE.colliders[entity];
        Transformation transformation0 = SCENE.transformations[entity];

#ifdef OPTIMIZED_COLLISIONS
        Array* tiles = &SCENE.entity_to_tiles[entity];
        static int collided_targets[MAX_N_ENTITIES];
        memset(collided_targets, 0, sizeof(collided_targets));
        for (int i_tile = 0; i_tile < tiles->length; ++i_tile) {
            int tile = array_get(tiles, i_tile);
            Array* targets = &SCENE.tile_to_entities[tile];
            for (int i_target = 0; i_target < targets->length;
                 ++i_target) {
                int target = array_get(targets, i_target);
#else
        for (int target = 0; target < SCENE.n_entities; ++target) {
#endif
                if (target <= entity || collided_targets[target] == 1) {
                    continue;
                }

                if (check_if_entities_can_collide(entity, target) == 0) {
                    continue;
                }

                collided_targets[target] = 1;
                Primitive primitive1 = SCENE.colliders[target];
                Transformation transformation1
                    = SCENE.transformations[target];
                Collision collision = {
                    .entity0 = entity, .entity1 = target};

                if (collide_primitives(
                        primitive0,
                        transformation0,
                        primitive1,
                        transformation1,
                        &collision
                    )) {
                    push_collision(collision);
                }
            }
        }
    }
}

static void resolve_mtv(int entity0, int entity1, Collision collision) {
    Vec2 mtv = collision.mtv;
    RigidBody* rb0 = &SCENE.rigid_bodies[entity0];
    RigidBody* rb1 = &SCENE.rigid_bodies[entity1];
    Transformation* transformation0 = &SCENE.transformations[entity0];
    Transformation* transformation1 = &SCENE.transformations[entity1];
    if (check_if_entities_can_collide(entity0, entity1) == 0) {
        fprintf(
            stderr,
            "ERROR: Trying to resolve a collision between 2 entities "
            "which can't collide with each other. It's a bug\n"
        );
        exit(1);
    }

    if (rb0->type == DYNAMIC_RIGID_BODY
        && rb1->type == DYNAMIC_RIGID_BODY) {
        update_position(
            entity0, add(transformation0->curr_position, scale(mtv, 0.5))
        );
        update_position(
            entity1, add(transformation1->curr_position, scale(mtv, -0.5))
        );
    } else if (rb0->type == DYNAMIC_RIGID_BODY) {
        update_position(entity0, add(transformation0->curr_position, mtv));
    } else if (rb1->type == DYNAMIC_RIGID_BODY) {
        update_position(
            entity1, add(transformation1->curr_position, flip(mtv))
        );
    } else {
        fprintf(
            stderr,
            "ERROR: Trying to resolve a collision between 2 non-dynamic "
            "rigid bodies. It's a bug\n"
        );
        exit(1);
    }
}

static void update_scores(int entity) {
    if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
        update_get_rb_collided_score(entity);
    }
}

static void resolve_collisions(int is_playing) {
    if (DEBUG.collisions.resolve || DEBUG.collisions.resolve_once) {
        DEBUG.collisions.resolve_once = 0;

        while (N_COLLISIONS > 0) {
            Collision collision = pop_collision();
            int entity0 = collision.entity0;
            int entity1 = collision.entity1;
            int can_collide = check_if_entities_can_collide(
                entity0, entity1
            );
            if (can_collide == 0) {
                fprintf(
                    stderr,
                    "ERROR: Trying to resolve a collision between 2 "
                    "entities which can't collide with each other. It's a "
                    "bug\n"
                );
                exit(1);
            }

            resolve_mtv(entity0, entity1, collision);
            if (is_playing) {
                update_scores(entity0);
                update_scores(entity1);
            }
        }
    } else {
        N_COLLISIONS = 0;
    }
}

void update_collisions(int is_playing) {
    // TODO: Another systems (e.g. vision or ray casting) may use the
    // tiling. So, it makes sense to factor out the update_tiling() into a
    // separate system
    PROFILE(update_tiling);
    PROFILE(compute_collisions);
    PROFILE(resolve_collisions, is_playing);
}

void render_colliders(void) {
    int required_component = TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        Transformation transformation = SCENE.transformations[entity];
        Primitive primitive = SCENE.colliders[entity];
        render_debug_primitive(
            transformation,
            primitive,
            SKYBLUE_COLOR,
            DEBUG_RENDER_LAYER,
            LINE
        );
    }
}
