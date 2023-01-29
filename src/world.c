#include "world.h"

#include "app.h"
#include "component.h"
#include "const.h"
#include "debug.h"
#include "math.h"
#include "system.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

World WORLD;
const char* COMPONENT_NAMES[N_COMPONENS] = {
    "Transformation",
    "Collider",
    "Primitive",
    "Material",
    "Kinematic",
    "Vision",
    "Rigid body",
    "Observable",
    "TTL",
    "Health",
    "Gun",
    "Bullet",
    "Owner"};

void init_world(void) {
    WORLD.n_entities = 0;
    WORLD.player = -1;
    WORLD.camera = -1;
}

void destroy_entity(int entity) {
    WORLD.components[entity] = 0;
}

void entity_disable_component(int entity, ComponentType type) {
    WORLD.components[entity] &= !type;
}

void entity_enable_component(int entity, ComponentType type) {
    WORLD.components[entity] |= type;
}

int get_entity_owner(int entity) {
    if (entity_has_component(entity, OWNER_COMPONENT)) {
        return WORLD.owners[entity];
    }
    return -1;
}

int entity_has_component(int entity, ComponentType type) {
    return (WORLD.components[entity] & type) == type;
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
        entity,
        PRIMITIVE_COMPONENT | TRANSFORMATION_COMPONENT | MATERIAL_COMPONENT
    );
}

int entity_can_be_damaged_by_bullet(int entity, int bullet) {
    int bullet_owner = get_entity_owner(bullet);
    int bullet_is_real = entity_has_component(bullet, BULLET_COMPONENT);
    int bullet_has_kinematic = entity_has_component(
        bullet, KINEMATIC_COMPONENT
    );
    int entity_has_health = entity_has_component(entity, HEALTH_COMPONENT);
    return bullet_owner != entity && bullet_is_real && bullet_has_kinematic
           && entity_has_health;
}

int bullet_can_be_destroyed_after_collision(int bullet, int target) {
    int bullet_owner = get_entity_owner(bullet);
    int bullet_is_real = entity_has_component(bullet, BULLET_COMPONENT);
    int target_is_not_bullet = !entity_has_component(
        target, BULLET_COMPONENT
    );
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
    WORLD.transformations[entity] = transformation;
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

    WORLD.transformations[entity] = transformation;
    WORLD.primitives[entity] = primitive;
    WORLD.colliders[entity] = collider;
    WORLD.materials[entity] = material;
    WORLD.kinematics[entity] = kinematic;
    WORLD.visions[entity] = vision;
    WORLD.guns[entity] = gun;
    WORLD.healths[entity] = health;

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
    WORLD.transformations[entity] = transformation;
    WORLD.primitives[entity] = primitive;
    WORLD.colliders[entity] = collider;
    WORLD.materials[entity] = material;
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
    WORLD.transformations[entity] = transformation;
    WORLD.primitives[entity] = primitive;
    WORLD.materials[entity] = material;
    WORLD.kinematics[entity] = kinematic;
    WORLD.ttls[entity] = ttl;
    WORLD.colliders[entity] = collider;
    WORLD.owners[entity] = owner;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_COMPONENT | COLLIDER_COMPONENT
                               | PRIMITIVE_COMPONENT | MATERIAL_COMPONENT
                               | TTL_COMPONENT | BULLET_COMPONENT
                               | OWNER_COMPONENT;
}

CameraFrustum get_camera_frustum() {
    CameraFrustum frustum;
    if (WORLD.camera != -1) {
        Transformation transformation
            = WORLD.transformations[WORLD.camera];
        float aspect_ratio = (float)APP.window_width / APP.window_height;
        float height = CAMERA_VIEW_WIDTH / aspect_ratio;
        Vec2 half_size = scale(vec2(CAMERA_VIEW_WIDTH, height), 0.5);
        frustum.bot_left = sub(transformation.position, half_size);
        frustum.top_right = add(transformation.position, half_size);
    }
    printf("bot_left:%f,%f\n", frustum.bot_left.x, frustum.bot_left.y);
    printf("top_right:%f,%f\n", frustum.top_right.x, frustum.top_right.y);

    return frustum;
}

static void update_entities_world_counter() {
    int n_entities = 0;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (WORLD.components[entity] != 0) {
            n_entities = entity + 1;
        } else if (DEBUG.picked_entity == entity) {
            DEBUG.picked_entity = -1;
        }
    }
    WORLD.n_entities = n_entities;
    DEBUG.general.n_entities = n_entities;
}

void update_world(float dt) {
    update_visions();

    if (DEBUG.is_playing) {
        update_camera();
        update_ttls(dt);
        update_healths();
        update_player();
        update_kinematics(dt);
        update_collisions();
        update_entities_world_counter();
    } else {
        update_cursor_picking();
    }
}
