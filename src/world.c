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

void destroy_entity(int entity) {
    WORLD.components[entity] = 0;
}

int get_entity_owner(int entity) {
    if (entity_has_owner(entity)) {
        return WORLD.owner[entity];
    }
    return -1;
}

int entity_has_component(int entity, ComponentType type) {
    return (WORLD.components[entity] & type) == type;
}

int entity_has_transformation(int entity) {
    return WORLD.components[entity] & TRANSFORMATION_COMPONENT;
}

int entity_has_ttl(int entity) {
    return WORLD.components[entity] & TTL_COMPONENT;
}

int entity_has_health(int entity) {
    return WORLD.components[entity] & HEALTH_COMPONENT;
}

int entity_has_rigid_body(int entity) {
    return WORLD.components[entity] & RIGID_BODY_COMPONENT;
}

int entity_has_kinematic(int entity) {
    return WORLD.components[entity] & KINEMATIC_COMPONENT;
}

int entity_has_bullet(int entity) {
    return WORLD.components[entity] & BULLET_COMPONENT;
}

int entity_has_owner(int entity) {
    return WORLD.components[entity] & OWNER_COMPONENT;
}

int entity_can_collide(int entity) {
    return entity_has_component(
        entity, TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT
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

int entity_can_be_damaged_by_bullet(int bullet, int target) {
    int bullet_owner = get_entity_owner(bullet);
    int bullet_is_real = entity_has_bullet(bullet);
    int bullet_has_kinematic = entity_has_kinematic(bullet);
    return bullet_owner != target && bullet_is_real && bullet_has_kinematic
           && entity_has_health(target);
}

int bullet_can_be_destroyed_after_collision(int bullet, int target) {
    int bullet_owner = get_entity_owner(bullet);
    int bullet_is_real = entity_has_bullet(bullet);
    int target_is_not_bullet = !entity_has_bullet(target);
    return bullet_owner != target && bullet_is_real
           && target_is_not_bullet;
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

int spawn_guy(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material,
    Kinematic kinematic,
    Vision vision,
    Gun gun,
    float health,
    int is_player
) {

    int entity = spawn_entity();
    if (is_player) {
        if (WORLD.player != -1) {
            fprintf(stderr, "ERROR: Only one player can be spawned\n");
            exit(1);
        }

        WORLD.player = entity;
    }

    WORLD.transformation[entity] = transformation;
    WORLD.primitive[entity] = primitive;
    WORLD.collider[entity] = collider;
    WORLD.material[entity] = material;
    WORLD.kinematic[entity] = kinematic;
    WORLD.vision[entity] = vision;
    WORLD.gun[entity] = gun;
    WORLD.health[entity] = health;

    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_COMPONENT | VISION_COMPONENT
                               | OBSERVABLE_COMPONENT | COLLIDER_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT | GUN_COMPONENT
                               | HEALTH_COMPONENT;

    return entity;
}

int spawn_obstacle(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material
) {
    int entity = spawn_entity();
    WORLD.transformation[entity] = transformation;
    WORLD.primitive[entity] = primitive;
    WORLD.collider[entity] = collider;
    WORLD.material[entity] = material;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | COLLIDER_COMPONENT | OBSERVABLE_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT;

    return entity;
}

int spawn_bullet(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material,
    Kinematic kinematic,
    float ttl,
    int owner
) {
    int entity = spawn_entity();
    WORLD.transformation[entity] = transformation;
    WORLD.primitive[entity] = primitive;
    WORLD.material[entity] = material;
    WORLD.kinematic[entity] = kinematic;
    WORLD.ttl[entity] = ttl;
    WORLD.collider[entity] = collider;
    WORLD.owner[entity] = owner;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_COMPONENT | COLLIDER_COMPONENT
                               | PRIMITIVE_COMPONENT | MATERIAL_COMPONENT
                               | TTL_COMPONENT | BULLET_COMPONENT
                               | OWNER_COMPONENT;
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

static void update_entities_world_counter() {
    int n_entities = 0;
    for (int e = 0; e < WORLD.n_entities; ++e) {
        if (WORLD.components[e] != 0) {
            n_entities = e + 1;
        }
    }
    WORLD.n_entities = n_entities;
}

static void check_entities_health() {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_health(entity)) {
            continue;
        }

        if (WORLD.health[entity] <= 0) {
            destroy_entity(entity);
        }
    }
}

static void update_entities_ttl(float dt) {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_ttl(entity)) {
            continue;
        }

        WORLD.ttl[entity] -= dt;
        if (WORLD.ttl[entity] < 0) {
            destroy_entity(entity);
        }
    }
}

void update_world(float dt) {
    dt /= 1000.0;

    update_entities_ttl(dt);
    check_entities_health();
    update_entities_world_counter();

    // Update player based on the input
    if (WORLD.player != -1) {
        Kinematic* kinematic = &WORLD.kinematic[WORLD.player];
        Transformation* transformation
            = &WORLD.transformation[WORLD.player];
        Vec2 velocity = {0.0, 0.0};

        velocity.y += 1.0 * APP.key_states[GLFW_KEY_W];
        velocity.y -= 1.0 * APP.key_states[GLFW_KEY_S];
        velocity.x -= 1.0 * APP.key_states[GLFW_KEY_A];
        velocity.x += 1.0 * APP.key_states[GLFW_KEY_D];
        if (length(velocity) > EPS) {
            velocity = scale(normalize(velocity), kinematic->max_speed);
        }
        kinematic->velocity = velocity;
        Vec2 look_at = screen_to_world(get_cursor_screen_pos());
        kinematic->orientation = atan2(
            look_at.y - transformation->position.y,
            look_at.x - transformation->position.x
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
                if (gun->last_time_shoot == 0
                    || (time_since_last_shoot > 1.0 / gun->fire_rate)) {
                    gun->last_time_shoot = APP.time;
                    Vec2 bullet_velocity = vec2(
                        cos(transformation->orientation),
                        sin(transformation->orientation)
                    );
                    bullet_velocity = scale(
                        bullet_velocity, gun->bullet.speed
                    );
                    Kinematic bullet_kinematic = {
                        bullet_velocity,
                        gun->bullet.speed,
                        transformation->orientation,
                        0.0};
                    Transformation bullet_transformation = *transformation;
                    float bullet_collider_length = gun->bullet.speed
                                                   / 30.0;
                    bullet_transformation = *transformation;
                    Vec2 bullet_position_offset = scale(
                        vec2(
                            cos(bullet_transformation.orientation),
                            sin(bullet_transformation.orientation)
                        ),
                        bullet_collider_length * 0.5
                    );
                    bullet_transformation.position = add(
                        bullet_transformation.position,
                        bullet_position_offset
                    );
                    Primitive bullet_primitive = circle_primitive(
                        circle(0.1)
                    );
                    Primitive bullet_collider = line_primitive(
                        line(vec2(bullet_collider_length, 0.0))
                    );
                    Material bullet_material = material(RED_COLOR);

                    spawn_bullet(
                        bullet_transformation,
                        bullet_primitive,
                        bullet_collider,
                        bullet_material,
                        bullet_kinematic,
                        gun->bullet.ttl,
                        WORLD.player
                    );
                }
            }
        }
    }

    // Observe world via vision component
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        observe_world(entity);
    }

    // Update positions of the kinematic entities
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        apply_kinematic(entity, dt);
    }

    // Collide entities with each other
    WORLD.n_collisions = 0;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        collide_with_world(entity);
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
