#include "../component.h"
#include "../editor.h"
#include "../math.h"
#include "../scene.h"

RayCastResult cast_ray(
    Vec2 start, Vec2 ray, int target_components, int ray_owner
) {
    RayCastResult result;
    result.entity = -1;
    int required_component = TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT
                             | target_components;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
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

    if (result.entity == -1) {
        result.position = add(start, ray);
    }

    return result;
}
