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
    WORLD.camera = -1;
}

int entity_has_component(int entity, ComponentType type) {
    return (WORLD.components[entity] & type) == type;
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

int spawn_camera(Transformation transformation) {
    int entity = -1;
    if (WORLD.camera != -1) {
        fprintf(stderr, "ERROR: Only one camera can be spawned\n");
        exit(1);
    }

    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
        WORLD.transformation[entity] = transformation;
        WORLD.components[entity] = TRANSFORMATION_COMPONENT;
    } else {
        fprintf(
            stderr,
            "ERROR: Can't spawn the camera, max number of entities has "
            "been reached\n"
        );
        exit(1);
    }

    WORLD.camera = entity;
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
        fprintf(stderr, "ERROR: Only one player can be spawned\n");
        exit(1);
    }
    WORLD.player = spawn_guy(
        transformation, primitive, material, kinematic, vision
    );
    return WORLD.player;
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
        fprintf(stderr, "ERROR: Can't spawn more guys\n");
        exit(1);
    }

    return entity;
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
        fprintf(stderr, "ERROR: Can't spawn more obstacles\n");
        exit(1);
    }

    return entity;
}

static Vec2 screen_to_world(Vec2 screen_pos) {
    Transformation t = WORLD.transformation[WORLD.camera];
    float aspect_ratio = (float)APP.window_width / APP.window_height;
    float camera_view_height = CAMERA_VIEW_WIDTH / aspect_ratio;
    float x = t.position.x + CAMERA_VIEW_WIDTH * (screen_pos.x - 0.5);
    float y = t.position.y + camera_view_height * (screen_pos.y - 0.5);
    return vec2(x, y);
}

void update_world(float dt) {
    dt /= 1000.0;

    // Update kinematic component based on the player input
    if (WORLD.player != -1) {
        Kinematic* k = &WORLD.kinematic[WORLD.player];
        Transformation* t = &WORLD.transformation[WORLD.player];
        Vec2 velocity = {0.0, 0.0};

        velocity.y += 1.0 * APP.key_states[GLFW_KEY_W];
        velocity.y -= 1.0 * APP.key_states[GLFW_KEY_S];
        velocity.x -= 1.0 * APP.key_states[GLFW_KEY_A];
        velocity.x += 1.0 * APP.key_states[GLFW_KEY_D];
        if (length(velocity) > EPS) {
            velocity = scale(normalize(velocity), k->max_speed);
        }
        k->velocity = velocity;
        Vec2 look_at = screen_to_world(get_cursor_screen_pos());
        k->rotation = atan2(look_at.y, look_at.x);
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

        float diff = get_orientations_diff(k->rotation, t->rotation);
        float step = k->rotation_speed * dt;
        // if (diff <= step) {
        //     t->rotation = k->rotation;
        // } else {
        //     float new_rotation0 = t->rotation + step;
        //     float new_rotation1 = t->rotation - step;
        //     float diff0 = get_orientations_diff(k->rotation,
        //     new_rotation0); float diff1 =
        //     get_orientations_diff(k->rotation, new_rotation1); if (diff0
        //     < diff1) {
        //         t->rotation = new_rotation0;
        //     } else {
        //         t->rotation = new_rotation1;
        //     }
        // }
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
