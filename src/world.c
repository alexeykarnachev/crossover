#include "world.h"

#include "app.h"
#include "collision/collision.h"
#include "const.h"
#include "debug/debug.h"
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
    dt /= 1000.0;

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
        if (length(m.direction) > EPS) {
            Vec2 t = scale(normalize(m.direction), m.speed * dt);
            transform_entity(e, t, 0.0);
        }
    }

    // Collide entities with each other
    WORLD.n_collisions = 0;
    for (int e0 = 0; e0 < WORLD.n_entities; ++e0) {
        for (int e1 = e0 + 1; e1 < WORLD.n_entities; ++e1) {
            Collision* collision = &WORLD.collisions[WORLD.n_collisions];
            WORLD.n_collisions += collide_entities(e0, e1, collision);
        }
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
    if (has_primitive) {
        translate_primitive(&WORLD.primitive[entity], translation);
    }

    if (has_collider) {
        translate_primitive(&WORLD.collider[entity], translation);
    }
}

