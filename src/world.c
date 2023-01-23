#include "world.h"

#include "app.h"
#include "collision.h"
#include "const.h"
#include "debug/debug.h"
#include "kinematic.h"
#include "math.h"
#include "transformation.h"
#include <math.h>
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

int entity_can_collide(int entity) {
    return entity_has_component(
        entity, COLLIDER_COMPONENT | TRANSFORMATION_COMPONENT
    );
}

int entity_can_observe(int entity) {
    return entity_has_component(
        entity, TRANSFORMATION_COMPONENT | VISION_COMPONENT
    );
}

int entity_can_be_rendered(int entity) {
    return entity_has_component(
        entity, PRIMITIVE_COMPONENT | TRANSFORMATION_COMPONENT
    );
}

int entity_can_be_observed(int entity) {
    return entity_has_component(
        entity,
        COLLIDER_COMPONENT | TRANSFORMATION_COMPONENT
            | OBSERVABLE_COMPONENT
    );
}

int spawn_guy(
    Transformation transformation,
    Primitive primitive,
    Material material,
    Kinematic kinematic,
    Vision vision
) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.transformation[entity] = transformation;
        WORLD.primitive[entity] = primitive;
        WORLD.material[entity] = material;
        WORLD.kinematic[entity] = kinematic;
        WORLD.vision[entity] = vision;
        WORLD.collider[entity] = primitive;
        WORLD.components[entity] = TRANSFORMATION_COMPONENT
                                   | KINEMATIC_COMPONENT | VISION_COMPONENT
                                   | OBSERVABLE_COMPONENT
                                   | COLLIDER_COMPONENT
                                   | RIGID_BODY_COMPONENT
                                   | PRIMITIVE_COMPONENT
                                   | MATERIAL_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more guys");
        exit(1);
    }

    return entity;
}

int spawn_player(
    Transformation transformation,
    Primitive primitive,
    Material material,
    Kinematic kinematic,
    Vision vision
) {
    if (WORLD.player != -1) {
        fprintf(stderr, "ERROR: Only one player can be spawned");
        exit(1);
    }
    WORLD.player = spawn_guy(
        transformation, primitive, material, kinematic, vision
    );
    return WORLD.player;
}

int spawn_obstacle(
    Transformation transformation, Primitive primitive, Material material
) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.transformation[entity] = transformation;
        WORLD.primitive[entity] = primitive;
        WORLD.material[entity] = material;
        WORLD.collider[entity] = primitive;
        WORLD.components[entity] = TRANSFORMATION_COMPONENT
                                   | COLLIDER_COMPONENT
                                   | OBSERVABLE_COMPONENT
                                   | RIGID_BODY_COMPONENT
                                   | PRIMITIVE_COMPONENT
                                   | MATERIAL_COMPONENT;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more obstacles");
        exit(1);
    }

    return entity;
}

void update_world(float dt) {
    dt /= 1000.0;

    // Update kinematic component based on the player input
    if (WORLD.player != -1) {
        Kinematic* k = &WORLD.kinematic[WORLD.player];
        Vec2 velocity = {0.0, 0.0};

        velocity.y += 1.0 * APP.key_states[GLFW_KEY_W];
        velocity.y -= 1.0 * APP.key_states[GLFW_KEY_S];
        velocity.x -= 1.0 * APP.key_states[GLFW_KEY_A];
        velocity.x += 1.0 * APP.key_states[GLFW_KEY_D];
        if (length(velocity) > EPS) {
            velocity = scale(normalize(velocity), k->max_speed);
        }
        k->velocity = velocity;
    }

    // Observe world via vision component
    WORLD.n_collisions = 0;
    for (int e = 0; e < WORLD.n_entities; ++e) {
        observe_world(e);
    }

    // Update positions of the kinematic entities
    for (int e = 0; e < WORLD.n_entities; ++e) {
        if (!entity_has_component(e, KINEMATIC_COMPONENT)) {
            continue;
        }
        Transformation* t = &WORLD.transformation[e];
        Kinematic* k = &WORLD.kinematic[e];
        t->position = add(t->position, scale(k->velocity, dt));
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
