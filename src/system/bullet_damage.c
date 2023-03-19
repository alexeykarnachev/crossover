#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../math.h"
#include "../renderer.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void update_bullets(float dt) {
    int required_component = TRANSFORMATION_COMPONENT | BULLET_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        int bullet_owner = -1;
        if (check_if_entity_has_component(entity, OWNER_COMPONENT)) {
            bullet_owner = SCENE.owners[entity];
        }
        Transformation* transformation = &SCENE.transformations[entity];
        Bullet bullet = SCENE.bullets[entity];
        Vec2 velocity = scale(
            get_orientation_vec(transformation->curr_orientation),
            bullet.speed
        );
        Vec2 ray = scale(velocity, dt);
        int target_required_component = TRANSFORMATION_COMPONENT
                                        | COLLIDER_COMPONENT
                                        | RIGID_BODY_COMPONENT;
        RayCastResult result = cast_ray(
            transformation->curr_position,
            ray,
            target_required_component,
            bullet_owner
        );
        int target = result.entity;
        if (target != -1) {
            if (check_if_entity_has_component(target, HEALTH_COMPONENT)) {
                float damage = length(velocity);
                Health* health = &SCENE.healths[target];
                health->curr_value -= damage;
                health->damage_dealler = bullet_owner;

                if (check_if_entity_has_component(
                        target, SCORER_COMPONENT
                    )) {
                    update_get_hit_score(target);
                }

                if (bullet_owner != -1
                    && check_if_entity_has_component(
                        bullet_owner, SCORER_COMPONENT
                    )) {
                    update_do_hit_score(bullet_owner);
                }
            }
            destroy_entity(entity);
        } else {
            Vec2 position = add(
                transformation->curr_position, scale(velocity, dt)
            );
            update_position(entity, position);
        }
    }
}

void render_bullets(float dt) {
    int required_component = TRANSFORMATION_COMPONENT | BULLET_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        Transformation transformation = SCENE.transformations[entity];
        Bullet bullet = SCENE.bullets[entity];
        Vec2 velocity = scale(
            get_orientation_vec(transformation.curr_orientation),
            bullet.speed
        );
        Vec2 ray = scale(velocity, dt);
        render_debug_line(
            transformation.curr_position,
            add(transformation.curr_position, ray),
            YELLOW_COLOR,
            DEBUG_RENDER_LAYER
        );
    }
}
