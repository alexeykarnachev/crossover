#include "world.h"

#include "./component/component.h"
#include "./system/system.h"
#include "app.h"
#include "const.h"
#include "debug/debug.h"
#include "math.h"
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

int entity_can_be_damaged_by_bullet(int entity, int bullet) {
    int bullet_owner = get_entity_owner(bullet);
    int bullet_is_real = entity_has_bullet(bullet);
    int bullet_has_kinematic = entity_has_kinematic(bullet);
    return bullet_owner != entity && bullet_is_real && bullet_has_kinematic
           && entity_has_health(entity);
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

static void update_entities_world_counter() {
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

    update_ttl(dt);
    update_health();
    update_player();
    update_vision();
    update_kinematic(dt);
    update_collision();

    update_entities_world_counter();
}
