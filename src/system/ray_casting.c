#include "../component.h"
#include "../editor.h"
#include "../math.h"
#include "../scene.h"
#include <string.h>

RayCastResult cast_ray(
    Vec2 start, Vec2 ray, int target_components, int ray_owner
) {
    int required_component = TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT
                             | target_components;
    RayCastResult result;
    result.entity = -1;
#ifdef OPTIMIZED_RAY_CASTING
    float left_x = min(start.x, start.x + ray.x);
    float right_x = max(start.x, start.x + ray.x);
    float top_y = max(start.y, start.y + ray.y);
    float bot_y = min(start.y, start.y + ray.y);

    Vec2 start_tile = get_tile_location_at(vec2(left_x, top_y));
    Vec2 last_tile = get_tile_location_at(vec2(right_x, bot_y));
    Vec2 curr_tile = start_tile;
    static Array targets;
    empty_array(&targets);

    // TODO: This tiles iterator repeats the code from the collisions
    // detection. Could be factored out
    do {
        int tile = curr_tile.y * N_X_SCENE_TILES + curr_tile.x;
        if (curr_tile.x != -1 && curr_tile.y != -1 && tile >= 0
            && tile < N_SCENE_TILES) {
            append_array(&targets, &SCENE.tile_to_entities[tile]);
        }

        if (curr_tile.x == last_tile.x) {
            curr_tile.x = start_tile.x;
            curr_tile.y += 1;
        } else {
            curr_tile.x += 1;
        }
    } while (curr_tile.x <= last_tile.x && curr_tile.y <= last_tile.y);

    static float seen_targets[MAX_N_ENTITIES];
    memset(seen_targets, 0, sizeof(seen_targets));
    for (int t = 0; t < targets.length; ++t) {
        int entity = (int)array_get(&targets, t);
        if (seen_targets[entity] == 1) {
            continue;
        }
        seen_targets[entity] = 1;
#else
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
#endif
        if (!check_if_entity_has_component(entity, required_component)
            || ray_owner == entity) {
            continue;
        }
        Transformation entity_transformation
            = SCENE.transformations[entity];
        Primitive entity_collider = SCENE.colliders[entity];

        Vec2 collision_position;
        int is_collided = 0;
        switch (entity_collider.type) {
            case CIRCLE_PRIMITIVE:
                is_collided = intersect_line_with_circle_nearest(
                    start,
                    add(start, ray),
                    entity_transformation.curr_position,
                    entity_collider.p.circle.radius,
                    &collision_position
                );
                break;
            default: {
                Vec2 vertices[MAX_N_POLYGON_VERTICES];
                int nv = get_primitive_vertices(entity_collider, vertices);
                apply_transformation(vertices, nv, entity_transformation);

                is_collided = intersect_line_with_polygon_nearest(
                    start,
                    add(start, ray),
                    vertices,
                    nv,
                    &collision_position
                );
            }
        }

        if (!is_collided) {
            continue;
        }

        float new_dist = dist(start, collision_position);
        if (result.entity == -1
            || new_dist < dist(start, result.position)) {
            result.position = collision_position;
            result.entity = entity;
        }
    }

    if (result.entity == -1) {
        result.position = add(start, ray);
    }

    return result;
}
