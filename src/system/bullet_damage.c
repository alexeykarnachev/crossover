#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void update_bullets(float dt) {
    int required_component = TRANSFORMATION_COMPONENT
                             | KINEMATIC_MOVEMENT_COMPONENT
                             | BULLET_COMPONENT;
    for (int bullet = 0; bullet < SCENE.n_entities; ++bullet) {
        if (!check_if_entity_has_component(bullet, required_component)) {
            continue;
        }

        int bullet_owner = -1;
        if (check_if_entity_has_component(bullet, OWNER_COMPONENT)) {
            bullet_owner = SCENE.owners[bullet];
        }
        Transformation transformation = SCENE.transformations[bullet];
        KinematicMovement movement = SCENE.kinematic_movements[bullet];
        Vec2 ray = scale(get_kinematic_velocity(movement), dt);
        int target_required_component = TRANSFORMATION_COMPONENT
                                        | COLLIDER_COMPONENT
                                        | RIGID_BODY_COMPONENT;
        RayCastResult result = cast_ray(
            transformation.position,
            ray,
            target_required_component,
            bullet_owner
        );
        int entity = result.entity;
        if (entity != -1) {
            if (check_if_entity_has_component(entity, HEALTH_COMPONENT)) {
                float damage = movement.speed;
                Health* health = &SCENE.healths[entity];
                health->health -= damage;
                health->damage_dealler = bullet_owner;

                if (check_if_entity_has_component(
                        entity, SCORER_COMPONENT
                    )) {
                    Scorer* scorer = &SCENE.scorers[entity];
                    scorer->score += scorer->weight.loose_health * damage;
                }

                if (bullet_owner != -1
                    && check_if_entity_has_component(
                        bullet_owner, SCORER_COMPONENT
                    )) {
                    Scorer* scorer = &SCENE.scorers[bullet_owner];
                    scorer->score += scorer->weight.hit_enemy * damage;
                }
            }
            destroy_entity(bullet);
        }
    }
}

void render_bullets(float dt) {
    int required_component = TRANSFORMATION_COMPONENT
                             | KINEMATIC_MOVEMENT_COMPONENT
                             | BULLET_COMPONENT;
    for (int bullet = 0; bullet < SCENE.n_entities; ++bullet) {
        if (!check_if_entity_has_component(bullet, required_component)) {
            continue;
        }

        Transformation transformation = SCENE.transformations[bullet];
        KinematicMovement movement = SCENE.kinematic_movements[bullet];
        Vec2 ray = scale(get_kinematic_velocity(movement), dt);
        render_debug_line(
            transformation.position,
            add(transformation.position, ray),
            YELLOW_COLOR,
            DEBUG_RENDER_LAYER
        );
    }
}
