#include "world.h"

#include "app.h"
#include "const.h"
#include "math.h"
#include "physics.h"
#include "stdio.h"

World WORLD;

void init_world(void) {
    WORLD.n_entities = 0;
    WORLD.player = -1;
}

int entity_has_component(int entity, ComponentType type) {
    return WORLD.components[entity] & type;
}

int spawn_guy(Primitive primitive, Physics physics) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.physics[entity] = physics;
        WORLD.collider[entity] = 1;
        WORLD.primitive[entity] = primitive;
        WORLD.components[entity] = PHYSICS_COMPONENT | COLLIDER_COMPONENT
                                   | PRIMITIVE_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more guys!");
    }

    return entity;
}

int spawn_obstacle(Primitive primitive) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.collider[entity] = 1;
        WORLD.primitive[entity] = primitive;
        WORLD.components[entity] = COLLIDER_COMPONENT
                                   | PRIMITIVE_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more obstacles!");
    }

    return entity;
}

void update_world(float dt) {
    if (WORLD.player != -1) {
        Physics* p = &WORLD.physics[WORLD.player];
        p->movement_direction = vec2(0.0, 0.0);
        p->movement_direction.y += 1.0 * APP.key_states[GLFW_KEY_W];
        p->movement_direction.y -= 1.0 * APP.key_states[GLFW_KEY_S];
        p->movement_direction.x -= 1.0 * APP.key_states[GLFW_KEY_A];
        p->movement_direction.x += 1.0 * APP.key_states[GLFW_KEY_D];
    }

    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (entity_has_component(
                entity, PRIMITIVE_COMPONENT | PHYSICS_COMPONENT
            )) {
            update_primitive_by_physics(
                &WORLD.primitive[entity], WORLD.physics[entity], dt
            );
        }
    }
}
