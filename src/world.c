#include "world.h"

#include "app.h"
#include "collision.h"
#include "const.h"
#include "debug/debug.h"
#include "math.h"
#include "movement.h"
#include <stdio.h>
#include <stdlib.h>

World WORLD;

void init_world(void) {
    WORLD.n_entities = 0;
    WORLD.n_collisions = 0;
    WORLD.player = -1;
}

int entity_has_component(int entity, ComponentType type) {
    return WORLD.components[entity] & type;
}

int spawn_guy(
    Primitive primitive,
    Material material,
    Movement movement,
    Vision vision
) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.primitive[entity] = primitive;
        WORLD.material[entity] = material;
        WORLD.movement[entity] = movement;
        WORLD.vision[entity] = vision;
        WORLD.collider[entity] = primitive;
        WORLD.components[entity] = MOVEMENT_COMPONENT | VISION_COMPONENT
                                   | OBSERVABLE_COMPONENT
                                   | COLLIDER_COMPONENT
                                   | RIGID_BODY_COMPONENT
                                   | PRIMITIVE_COMPONENT
                                   | MATERIAL_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more guys!");
        exit(1);
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
                                   | OBSERVABLE_COMPONENT
                                   | RIGID_BODY_COMPONENT
                                   | PRIMITIVE_COMPONENT
                                   | MATERIAL_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more obstacles!");
        exit(1);
    }

    return entity;
}

void update_world(float dt) {
    dt /= 1000.0;

    // Update movement component based on the player input
    if (WORLD.player != -1) {
        Movement* m = &WORLD.movement[WORLD.player];
        m->direction = vec2(0.0, 0.0);
        m->direction.y += 1.0 * APP.key_states[GLFW_KEY_W];
        m->direction.y -= 1.0 * APP.key_states[GLFW_KEY_S];
        m->direction.x -= 1.0 * APP.key_states[GLFW_KEY_A];
        m->direction.x += 1.0 * APP.key_states[GLFW_KEY_D];
    }

    // Observe world via vision component
    WORLD.n_collisions = 0;
    for (int e = 0; e < WORLD.n_entities; ++e) {
        observe_world(e);
    }

    // Update positions of the movable entities
    for (int e = 0; e < WORLD.n_entities; ++e) {
        move_entity(e, dt);
    }

    // Collide entities with each other
    WORLD.n_collisions = 0;
    for (int e = 0; e < WORLD.n_entities; ++e) {
        collide_with_world(e);
    }

    // Resolve collisions
    if (DEBUG.collisions.resolve || DEBUG.collisions.resolve_once) {
        DEBUG.collisions.resolve_once = 0;

        for (int i = 0; i < WORLD.n_collisions; ++i) {
            Collision collision = WORLD.collisions[i];
            resolve_collision(collision);
        }
    }
}

void transform_entity(int entity, Vec2 translation, float angle) {
    int has_primitive = entity_has_component(entity, PRIMITIVE_COMPONENT);
    int has_collider = entity_has_component(entity, COLLIDER_COMPONENT);
    int has_vision = entity_has_component(entity, VISION_COMPONENT);
    if (has_primitive) {
        Primitive* p = &WORLD.primitive[entity];
        translate_primitive(p, translation);
        rotate_primitive(p, angle);
    }

    if (has_collider) {
        Primitive* p = &WORLD.collider[entity];
        translate_primitive(p, translation);
        rotate_primitive(p, angle);
    }

    if (has_vision) {
        Vision* v = &WORLD.vision[entity];
        translate_vision(v, translation);
        rotate_vision(v, angle);
    }
}
