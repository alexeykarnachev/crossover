#include "world.h"

#include "app.h"
#include "collision.h"
#include "const.h"
#include "debug/debug.h"
#include "kinematic.h"
#include "math.h"
#include "transformation.h"
#include "ttl.h"
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

int entity_has_ttl(int entity) {
    return WORLD.components[entity] & TTL_COMPONENT;
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

int entity_can_apply_kinematic(int entity) {
    return entity_has_component(
        entity, TRANSFORMATION_COMPONENT | KINEMATIC_COMPONENT
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

static int spawn_entity() {
    for (int entity = 0; entity < MAX_N_ENTITIES; ++entity) {
        if (WORLD.components[entity] == 0) {
            WORLD.n_entities = max(WORLD.n_entities, entity + 1);
            return entity;
        }
    }

    fprintf(stderr, "ERROR: Can't spawn more entities\n");
    exit(1);
}

int spawn_camera(Transformation transformation) {
    if (WORLD.camera != -1) {
        fprintf(stderr, "ERROR: Only one camera can be spawned\n");
        exit(1);
    }

    int entity = spawn_entity();
    WORLD.transformation[entity] = transformation;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT;

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

    int entity = spawn_guy(
        transformation, primitive, material, kinematic, vision
    );

    WORLD.player = entity;
    return entity;
}

int spawn_guy(
    Transformation transformation,
    Primitive primitive,
    Material material,
    Kinematic kinematic,
    Vision vision
) {
    int entity = spawn_entity();
    WORLD.transformation[entity] = transformation;
    WORLD.primitive[entity] = primitive;
    WORLD.material[entity] = material;
    WORLD.kinematic[entity] = kinematic;
    WORLD.vision[entity] = vision;
    WORLD.collider[entity] = primitive;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_COMPONENT | VISION_COMPONENT
                               | OBSERVABLE_COMPONENT | COLLIDER_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT;

    return entity;
}

int spawn_obstacle(
    Transformation transformation, Primitive primitive, Material material
) {
    int entity = spawn_entity();
    WORLD.transformation[entity] = transformation;
    WORLD.primitive[entity] = primitive;
    WORLD.material[entity] = material;
    WORLD.collider[entity] = primitive;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | COLLIDER_COMPONENT | OBSERVABLE_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT;

    return entity;
}

int spawn_bullet(
    Transformation transformation,
    Primitive primitive,
    Material material,
    Kinematic kinematic,
    TTL ttl
) {
    int entity = spawn_entity();
    WORLD.transformation[entity] = transformation;
    WORLD.primitive[entity] = primitive;
    WORLD.material[entity] = material;
    WORLD.kinematic[entity] = kinematic;
    WORLD.ttl[entity] = ttl;
    WORLD.collider[entity] = primitive;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_COMPONENT | COLLIDER_COMPONENT
                               | PRIMITIVE_COMPONENT | MATERIAL_COMPONENT
                               | TTL_COMPONENT;
}

void set_gun(int entity, Gun gun) {
    WORLD.components[entity] |= GUN_COMPONENT;
    WORLD.gun[entity] = gun;
}

CameraFrustum get_camera_frustum() {
    CameraFrustum frustum;
    if (WORLD.camera != -1) {
        Transformation t = WORLD.transformation[WORLD.camera];
        float aspect_ratio = (float)APP.window_width / APP.window_height;
        float height = CAMERA_VIEW_WIDTH / aspect_ratio;
        Vec2 half_size = scale(vec2(CAMERA_VIEW_WIDTH, height), 0.5);
        frustum.bot_left = sub(t.position, half_size);
        frustum.top_right = add(t.position, half_size);
    }

    return frustum;
}

static Vec2 screen_to_world(Vec2 screen_pos) {
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;
    return vec2(x, y);
}

static void update_n_entities() {
    int n_entities = 0;
    for (int e = 0; e < WORLD.n_entities; ++e) {
        if (WORLD.components[e] != 0) {
            n_entities = e + 1;
        }
    }
    WORLD.n_entities = n_entities;
}

void update_world(float dt) {
    dt /= 1000.0;
    update_n_entities();

    // Update player based on the input
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
        k->orientation = atan2(
            look_at.y - t->position.y, look_at.x - t->position.x
        );

        DEBUG.general.look_at = look_at;
        if (DEBUG.shading.look_at) {
            render_debug_circle(look_at, 0.1, RED_COLOR);
        }

        if (APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
            if (entity_has_component(WORLD.player, GUN_COMPONENT)) {
                Gun* gun = &WORLD.gun[WORLD.player];
                float time_since_last_shoot
                    = (APP.time - gun->last_time_shoot);
                if (time_since_last_shoot > 1.0 / gun->fire_rate) {
                    gun->last_time_shoot = APP.time;
                    Vec2 velocity = vec2(
                        cos(t->orientation), sin(t->orientation)
                    );
                    velocity = scale(velocity, gun->bullet.base_speed);
                    spawn_bullet(
                        *t,
                        circle_primitive(circle(gun->bullet.radius)),
                        gun->bullet.material,
                        kinematic(velocity, gun->bullet.base_speed, 0.0),
                        gun->bullet.ttl
                    );
                }
            }
        }
    }

    // Observe world via vision component
    WORLD.n_collisions = 0;
    for (int e = 0; e < WORLD.n_entities; ++e) {
        observe_world(e);
    }

    // Update positions of the kinematic entities
    for (int e = 0; e < WORLD.n_entities; ++e) {
        apply_kinematic(e, dt);
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

    // Destroy TTL-ed entities
    for (int e = 0; e < WORLD.n_entities; ++e) {
        apply_ttl(e, dt);
    }
}
