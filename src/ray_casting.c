#include "ray_casting.h"

#include "math.h"
#include "primitive.h"
#include "transformation.h"
#include "world.h"

RayCastResult cast_ray(
    Vec2 start, Line ray, int target_components, int ray_owner
) {
    Transformation ray_transformation = {start, 0.0};

    RayCastResult result;
    result.entity = -1;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_can_collide(entity)
            || !entity_has_component(entity, target_components)
            || ray_owner == entity) {
            continue;
        }
        Transformation entity_transformation
            = WORLD.transformation[entity];
        Primitive entity_collider = WORLD.collider[entity];

        Vec2 collision_position;
        int is_collided = 0;
        switch (entity_collider.type) {
            case CIRCLE_PRIMITIVE:
                is_collided = intersect_line_with_circle_nearest(
                    start,
                    add(start, ray.b),
                    entity_transformation.position,
                    entity_collider.p.circle.radius,
                    &collision_position
                );
                break;
            default: {
                Vec2 vertices[4];
                int nv = get_primitive_vertices(
                    entity_collider, entity_transformation, vertices
                );
                is_collided = intersect_line_with_polygon_nearest(
                    start,
                    add(start, ray.b),
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
