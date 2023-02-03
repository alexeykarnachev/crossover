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
#include <string.h>

World WORLD;
const char* COMPONENT_NAMES[N_COMPONENTS] = {
    "Transformation",
    "Collider",
    "Primitive",
    "Render layer",
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

Vec2 get_cursor_world_pos(void) {
    Vec2 screen_pos = get_cursor_screen_pos();
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;

    Transformation camera = WORLD.transformations[WORLD.camera];
    Vec2 position = rotate(vec2(x, y), vec2(0.0, 0.0), camera.orientation);
    return position;
}

void init_world(void) {
    WORLD.n_entities = 0;
    WORLD.player = -1;
    WORLD.camera = -1;
    WORLD.camera_view_width = CAMERA_VIEW_WIDTH;
}

void destroy_entity(int entity) {
    WORLD.components[entity] = 0;
    if (DEBUG.picked_entity.entity == entity) {
        DEBUG.picked_entity.entity = -1;
    }
}

int entity_is_alive(int entity) {
    return WORLD.components[entity] != 0;
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

static int spawn_entity(const char* name) {
    if (strlen(name) > MAX_ENTITY_NAME_SIZE) {
        fprintf(
            stderr,
            "ERROR: Max. entity name can't be larget than %d\n",
            MAX_ENTITY_NAME_SIZE
        );
        exit(1);
    }

    for (int entity = 0; entity < MAX_N_ENTITIES; ++entity) {
        if (WORLD.components[entity] == 0) {
            WORLD.names[entity] = name;
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

    int entity = spawn_entity("Camera");
    WORLD.transformations[entity] = transformation;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT;

    WORLD.camera = entity;
    return entity;
}

int spawn_renderable_guy(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material,
    float render_layer,
    Kinematic kinematic,
    Vision vision,
    Gun gun,
    float health,
    int is_player
) {
    int entity = spawn_entity("Guy");
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
    WORLD.render_layers[entity] = render_layer;
    WORLD.kinematics[entity] = kinematic;
    WORLD.visions[entity] = vision;
    WORLD.guns[entity] = gun;
    WORLD.healths[entity] = health;

    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_COMPONENT | VISION_COMPONENT
                               | OBSERVABLE_COMPONENT | COLLIDER_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT | GUN_COMPONENT
                               | HEALTH_COMPONENT | RENDER_LAYER_COMPONENT;

    return entity;
}

int spawn_renderable_obstacle(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material,
    float render_layer
) {
    int entity = spawn_entity("Obstacle");
    WORLD.transformations[entity] = transformation;
    WORLD.primitives[entity] = primitive;
    WORLD.colliders[entity] = collider;
    WORLD.materials[entity] = material;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | COLLIDER_COMPONENT | OBSERVABLE_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT
                               | RENDER_LAYER_COMPONENT;

    return entity;
}

int spawn_bullet(
    Transformation transformation,
    Kinematic kinematic,
    float ttl,
    int owner
) {
    int entity = spawn_entity("Bullet");
    WORLD.transformations[entity] = transformation;
    WORLD.kinematics[entity] = kinematic;
    WORLD.ttls[entity] = ttl;
    WORLD.owners[entity] = owner;
    WORLD.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_COMPONENT | TTL_COMPONENT
                               | BULLET_COMPONENT | OWNER_COMPONENT;
}

int spawn_default_renderable_guy(Transformation transformation) {
    return spawn_renderable_guy(
        transformation,
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_material(REDWOOD_COLOR),
        0.0,
        init_kinematic(5.0, 2.0 * PI),
        init_vision(0.5 * PI, 10.0, 32),
        init_gun(4.0, 50.0, 1.0),
        1000.0,
        0
    );
}

int spawn_default_renderable_circle_obstacle(Transformation transformation
) {
    return spawn_renderable_obstacle(
        transformation,
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_renderable_rectangle_obstacle(
    Transformation transformation
) {
    return spawn_renderable_obstacle(
        transformation,
        init_default_rectangle_primitive(),
        init_default_rectangle_primitive(),
        init_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_renderable_line_obstacle(Transformation transformation) {
    return spawn_renderable_obstacle(
        transformation,
        init_default_line_primitive(),
        init_default_line_primitive(),
        init_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_renderable_polygon_obstacle(Transformation transformation
) {
    return spawn_renderable_obstacle(
        transformation,
        init_default_polygon_primitive(),
        init_default_polygon_primitive(),
        init_material(GRAY_COLOR),
        0.0
    );
}

CameraFrustum get_camera_frustum() {
    CameraFrustum frustum;
    if (WORLD.camera != -1) {
        Transformation transformation
            = WORLD.transformations[WORLD.camera];
        float aspect_ratio = (float)APP.window_width / APP.window_height;
        float height = WORLD.camera_view_width / aspect_ratio;
        Vec2 half_size = scale(vec2(WORLD.camera_view_width, height), 0.5);
        frustum.bot_left = sub(transformation.position, half_size);
        frustum.top_right = add(transformation.position, half_size);
    }

    return frustum;
}

void center_camera_on_entity(int entity) {
    Vec2 entity_position = WORLD.transformations[entity].position;
    WORLD.transformations[WORLD.camera].position = entity_position;
}

static void update_entities_world_counter() {
    int n_entities = 0;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (WORLD.components[entity] != 0) {
            n_entities = entity + 1;
        } else if (DEBUG.picked_entity.entity == entity) {
            DEBUG.picked_entity.entity = -1;
        }
    }
    WORLD.n_entities = n_entities;
    DEBUG.general.n_entities = n_entities;
}

void update_world(float dt) {
    update_visions();
    update_camera();

    if (DEBUG.is_playing) {
        update_ttls(dt);
        update_healths();
        update_player();
        update_bullets(dt);
        update_kinematics(dt);
        update_entities_world_counter();
    } else {
        update_entity_picking();
        update_entity_dragging();
    }

    update_collisions();
}
