#include "world.h"

#include "app.h"
#include "collision.h"
#include "const.h"
#include "math.h"
#include "movement.h"
#include "stdio.h"

World WORLD;

void init_world(void) {
    WORLD.n_entities = 0;
    WORLD.n_collisions = 0;
    WORLD.player = -1;
}

int entity_has_component(int entity, ComponentType type) {
    return WORLD.components[entity] & type;
}

int spawn_guy(Primitive primitive, Material material, Movement movement) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.primitive[entity] = primitive;
        WORLD.material[entity] = material;
        WORLD.movement[entity] = movement;
        WORLD.collider[entity] = primitive;
        WORLD.components[entity] = MOVEMENT_COMPONENT | COLLIDER_COMPONENT
                                   | RIGID_BODY_COMPONENT
                                   | PRIMITIVE_COMPONENT
                                   | MATERIAL_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more guys!");
    }

    return entity;
}

int spawn_obstacle(Primitive primitive, Material material) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.primitive[entity] = primitive;
        WORLD.material[entity] = material;
        WORLD.collider[entity] = primitive;
        WORLD.components[entity] = COLLIDER_COMPONENT
                                   | RIGID_BODY_COMPONENT
                                   | PRIMITIVE_COMPONENT
                                   | MATERIAL_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more obstacles!");
    }

    return entity;
}

void update_world(float dt) {
    if (WORLD.player != -1) {
        Movement* p = &WORLD.movement[WORLD.player];
        p->direction = vec2(0.0, 0.0);
        p->direction.y += 1.0 * APP.key_states[GLFW_KEY_W];
        p->direction.y -= 1.0 * APP.key_states[GLFW_KEY_S];
        p->direction.x -= 1.0 * APP.key_states[GLFW_KEY_A];
        p->direction.x += 1.0 * APP.key_states[GLFW_KEY_D];
    }

    // Update positions of the movable entities
    for (int e = 0; e < WORLD.n_entities; ++e) {
        if (!entity_has_component(e, MOVEMENT_COMPONENT)) {
            continue;
        }

        Movement m = WORLD.movement[e];
        int has_primitive = entity_has_component(e, PRIMITIVE_COMPONENT);
        int has_collider = entity_has_component(e, COLLIDER_COMPONENT);
        if (has_primitive) {
            move_primitive(&WORLD.primitive[e], m, dt);
        }

        if (has_collider) {
            move_primitive(&WORLD.collider[e], m, dt);
        }
    }

    // Collide movable entities with the world
    WORLD.n_collisions = 0;
    for (int e0 = 0; e0 < WORLD.n_entities; ++e0) {
        if (!entity_has_component(e0, COLLIDER_COMPONENT)
            || !entity_has_component(e0, MOVEMENT_COMPONENT)) {
            continue;
        }
        for (int e1 = e0 + 1; e1 < WORLD.n_entities; ++e1) {
            if (!entity_has_component(e1, COLLIDER_COMPONENT)) {
                continue;
            }

            Collision* collision = &WORLD.collisions[WORLD.n_collisions];
            collision->entity0 = e0;
            collision->entity1 = e1;
            WORLD.n_collisions += collide_primitives(
                WORLD.collider[e0], WORLD.collider[e1], &collision->type
            );
        }
    }
}
