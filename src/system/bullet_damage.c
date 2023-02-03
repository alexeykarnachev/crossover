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

void update_bullets(float dt) {
    for (int bullet = 0; bullet < WORLD.n_entities; ++bullet) {
        if (!entity_has_component(bullet, KINEMATIC_BULLET_COMPONENT)) {
            continue;
        }

        int owner = WORLD.owners[bullet];
        Transformation* transformation = &WORLD.transformations[bullet];
        Kinematic* kinematic = &WORLD.kinematics[bullet];
        Vec2 ray = scale(kinematic->velocity, dt);
        RayCastResult result = cast_ray(
            transformation->position,
            ray,
            DAMAGEABLE_BY_BULLET_COMPONENT,
            owner
        );
        int entity = result.entity;
        if (entity != -1) {
            if (entity_has_component(entity, HEALTH_COMPONENT)) {
                WORLD.healths[entity] -= get_kinematic_damage(*kinematic);
            }
            destroy_entity(bullet);
        }
    }
}

void render_bullets(float dt) {
    for (int bullet = 0; bullet < WORLD.n_entities; ++bullet) {
        if (!entity_has_component(bullet, KINEMATIC_BULLET_COMPONENT)) {
            continue;
        }

        Transformation transformation = WORLD.transformations[bullet];
        Kinematic kinematic = WORLD.kinematics[bullet];
        Vec2 ray = scale(kinematic.velocity, dt);
        render_debug_line(
            transformation.position,
            add(transformation.position, ray),
            YELLOW_COLOR,
            DEBUG_RENDER_LAYER
        );
    }
}
