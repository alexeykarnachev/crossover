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
    for (int bullet = 0; bullet < SCENE.n_entities; ++bullet) {
        if (!check_if_entity_has_component(
                bullet, KINEMATIC_BULLET_COMPONENT
            )) {
            continue;
        }

        int owner = -1;
        if (check_if_entity_has_component(bullet, OWNER_COMPONENT)) {
            owner = SCENE.owners[bullet];
        }
        Transformation* transformation = &SCENE.transformations[bullet];
        Kinematic* kinematic = &SCENE.kinematics[bullet];
        Vec2 ray = scale(kinematic->velocity, dt);
        RayCastResult result = cast_ray(
            transformation->position,
            ray,
            DAMAGEABLE_BY_BULLET_COMPONENT,
            owner
        );
        int entity = result.entity;
        if (entity != -1) {
            if (check_if_entity_has_component(entity, HEALTH_COMPONENT)) {
                SCENE.healths[entity] -= get_kinematic_damage(*kinematic);
            }
            destroy_entity(bullet);
        }
    }
}

void render_bullets(float dt) {
    for (int bullet = 0; bullet < SCENE.n_entities; ++bullet) {
        if (!check_if_entity_has_component(
                bullet, KINEMATIC_BULLET_COMPONENT
            )) {
            continue;
        }

        Transformation transformation = SCENE.transformations[bullet];
        Kinematic kinematic = SCENE.kinematics[bullet];
        Vec2 ray = scale(kinematic.velocity, dt);
        render_debug_line(
            transformation.position,
            add(transformation.position, ray),
            YELLOW_COLOR,
            DEBUG_RENDER_LAYER
        );
    }
}
