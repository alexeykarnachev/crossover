#include "world.h"

#include "app.h"
#include "const.h"
#include "math.h"
#include "movement.h"
#include "stdio.h"
#include "transformation.h"

World WORLD;

void init_world(void) {
    WORLD.n_entities = 0;
    WORLD.player = -1;
}

int entity_has_component(int entity, Component component) {
    return WORLD.components[entity] & component;
}

int spawn_guy(Transformation t, Movement m) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more guys!");
    }

    WORLD.transformation[entity] = t;
    WORLD.movement[entity] = m;
    WORLD.collider[entity] = 1;
    WORLD.circle[entity] = 1;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | MOVEMENT_COMPONENT | COLLIDER_COMPONENT
                               | CIRCLE_COMPONENT;

    return entity;
}

void update_world(float dt) {
    if (WORLD.player != -1) {
        Movement* m = &WORLD.movement[WORLD.player];
        m->target_speed = m->max_speed;
        if (APP.key_states[GLFW_KEY_W]) {
            m->target_orientation = 0.0;
        } else if (APP.key_states[GLFW_KEY_S]) {
            m->target_orientation = PI;
        } else {
            m->target_speed = 0.0;
        }
    }

    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (entity_has_component(entity, MOVEMENT_COMPONENT)) {
            update_movement(&WORLD.movement[entity], dt);
        }
    }

    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (entity_has_component(
                entity, TRANSFORMATION_COMPONENT | MOVEMENT_COMPONENT
            )) {
            update_transformation_by_movement(
                &WORLD.transformation[entity], WORLD.movement[entity], dt
            );
        }
    }
}
