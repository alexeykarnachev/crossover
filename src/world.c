#include "world.h"

#include "app.h"
#include "const.h"
#include "math.h"
#include "physics.h"
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

int spawn_guy(Transformation t, Physics p) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more guys!");
    }

    WORLD.transformation[entity] = t;
    WORLD.physics[entity] = p;
    WORLD.collider[entity] = 1;
    WORLD.circle[entity] = 1;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT | PHYSICS_COMPONENT
                               | COLLIDER_COMPONENT | CIRCLE_COMPONENT;

    return entity;
}

void update_world(float dt) {
    if (WORLD.player != -1) {
        Physics* p = &WORLD.physics[WORLD.player];
        int w = APP.key_states[GLFW_KEY_W];
        int s = APP.key_states[GLFW_KEY_S];
        int a = APP.key_states[GLFW_KEY_A];
        int d = APP.key_states[GLFW_KEY_D];

        if (w && d) {
            p->movement_direction = normalize_vec2(vec2(1.0, 1.0));
        } else if (w && a) {
            p->movement_direction = normalize_vec2(vec2(-1.0, 1.0));
        } else if (s && a) {
            p->movement_direction = normalize_vec2(vec2(-1.0, -1.0));
        } else if (s && d) {
            p->movement_direction = normalize_vec2(vec2(1.0, -1.0));
        } else if (w) {
            p->movement_direction = vec2(0.0, 1.0);
        } else if (s) {
            p->movement_direction = vec2(0.0, -1.0);
        } else if (a) {
            p->movement_direction = vec2(-1.0, 0.0);
        } else if (d) {
            p->movement_direction = vec2(1.0, 0.0);
        } else {
            p->movement_direction = vec2(0.0, 0.0);
        }
    }

    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (entity_has_component(
                entity, TRANSFORMATION_COMPONENT | PHYSICS_COMPONENT
            )) {
            update_transformation_by_physics(
                &WORLD.transformation[entity], WORLD.physics[entity], dt
            );
        }
    }
}
