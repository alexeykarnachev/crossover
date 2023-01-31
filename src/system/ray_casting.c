#include "../component.h"
#include "../math.h"
#include "../world.h"

RayCastResult cast_ray(
    Vec2 start, Vec2 ray, int target_components, int ray_owner
) {
    RayCastResult result;
    result.entity = -1;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_can_collide(entity)
            || !entity_has_component(entity, target_components)
            || ray_owner == entity) {
            continue;
        }
        Transformation entity_transformation
            = WORLD.transformations[entity];
        Primitive entity_collider = WORLD.colliders[entity];

        Vec2 collision_position;
        int is_collided = 0;
        switch (entity_collider.type) {
            case CIRCLE_PRIMITIVE:
                is_collided = intersect_line_with_circle_nearest(
                    start,
                    add(start, ray),
                    entity_transformation.position,
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

    return result;
}
