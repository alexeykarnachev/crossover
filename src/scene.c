#include "scene.h"

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

Scene SCENE;

Vec2 get_cursor_scene_pos(void) {
    Vec2 screen_pos = get_cursor_screen_pos();
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;

    Transformation camera = SCENE.transformations[SCENE.camera];
    Vec2 position = rotate(vec2(x, y), vec2(0.0, 0.0), camera.orientation);
    return position;
}

void reset_scene(void) {
    SCENE.version = SCENE_VERSION;
    SCENE.n_entities = 0;
    SCENE.player = -1;
    memset(SCENE.components, 0, sizeof(uint64_t) * MAX_N_ENTITIES);
    reset_camera();
}

int save_scene(const char* file_path) {
    FILE* fp = fopen(file_path, "wb");
    if (!fp) {
        fprintf(
            stderr, "ERROR: Can't save Scene to the file: %s\n", file_path
        );
        exit(1);
    }

    fwrite(&SCENE.version, sizeof(int), 1, fp);
    fwrite(&SCENE.n_entities, sizeof(int), 1, fp);
    fwrite(SCENE.components, sizeof(uint64_t), SCENE.n_entities, fp);

    for (int i = 0; i < SCENE.n_entities; ++i) {
        int name_len = strlen(SCENE.names[i]) + 1;
        fwrite(&name_len, sizeof(int), 1, fp);
        fwrite(SCENE.names[i], sizeof(char), name_len, fp);
    }

    fwrite(
        SCENE.transformations, sizeof(Transformation), SCENE.n_entities, fp
    );

    fwrite(SCENE.kinematics, sizeof(Kinematic), SCENE.n_entities, fp);
    fwrite(SCENE.visions, sizeof(Vision), SCENE.n_entities, fp);
    fwrite(SCENE.colliders, sizeof(Primitive), SCENE.n_entities, fp);
    fwrite(SCENE.primitives, sizeof(Primitive), SCENE.n_entities, fp);
    fwrite(SCENE.materials, sizeof(Material), SCENE.n_entities, fp);
    fwrite(SCENE.guns, sizeof(Gun), SCENE.n_entities, fp);
    fwrite(SCENE.ttls, sizeof(float), SCENE.n_entities, fp);
    fwrite(SCENE.healths, sizeof(float), SCENE.n_entities, fp);
    fwrite(SCENE.render_layers, sizeof(float), SCENE.n_entities, fp);
    fwrite(SCENE.owners, sizeof(int), SCENE.n_entities, fp);
    fwrite(&SCENE.camera, sizeof(int), 1, fp);
    fwrite(&SCENE.player, sizeof(int), 1, fp);
    fwrite(&SCENE.camera_view_width, sizeof(float), 1, fp);

    fclose(fp);
    return 1;
}

int load_scene(const char* file_path) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Can't load Scene file: %s\n", file_path);
        exit(1);
    }

    fread(&SCENE.version, sizeof(int), 1, fp);
    if (SCENE.version != SCENE_VERSION) {
        fprintf(
            stderr,
            "ERROR: Scene version %d is not compatible with the engine, "
            "expecting the version %d\n",
            SCENE.version,
            SCENE_VERSION
        );
        exit(1);
    }

    fread(&SCENE.n_entities, sizeof(int), 1, fp);
    fread(SCENE.components, sizeof(uint64_t), SCENE.n_entities, fp);

    for (int i = 0; i < SCENE.n_entities; ++i) {
        uint32_t name_len;
        fread(&name_len, sizeof(uint32_t), 1, fp);

        char* buffer = (char*)malloc(name_len + 1);
        fread(buffer, sizeof(char), name_len, fp);
        buffer[name_len] = '\0';

        SCENE.names[i] = buffer;
    }

    fread(
        SCENE.transformations, sizeof(Transformation), SCENE.n_entities, fp
    );
    fread(SCENE.kinematics, sizeof(Kinematic), SCENE.n_entities, fp);
    fread(SCENE.visions, sizeof(Vision), SCENE.n_entities, fp);
    fread(SCENE.colliders, sizeof(Primitive), SCENE.n_entities, fp);
    fread(SCENE.primitives, sizeof(Primitive), SCENE.n_entities, fp);
    fread(SCENE.materials, sizeof(Material), SCENE.n_entities, fp);
    fread(SCENE.guns, sizeof(Gun), SCENE.n_entities, fp);
    fread(SCENE.ttls, sizeof(float), SCENE.n_entities, fp);
    fread(SCENE.healths, sizeof(float), SCENE.n_entities, fp);
    fread(SCENE.render_layers, sizeof(float), SCENE.n_entities, fp);
    fread(SCENE.owners, sizeof(int), SCENE.n_entities, fp);
    fread(&SCENE.camera, sizeof(int), 1, fp);
    fread(&SCENE.player, sizeof(int), 1, fp);
    fread(&SCENE.camera_view_width, sizeof(float), 1, fp);

    fclose(fp);
    return 1;
}

void destroy_entity(int entity) {
    SCENE.components[entity] = 0;
    if (DEBUG.picked_entity.entity == entity) {
        pick_entity(-1);
    }

    if (DEBUG.entity_to_copy == entity) {
        DEBUG.entity_to_copy = -1;
    }

    // The entity is no longer the owner of any other entity
    for (int i = 0; i < SCENE.n_entities; ++i) {
        int owner = SCENE.owners[i];
        if (owner == entity) {
            SCENE.owners[i] = -1;
        }
    }
}

int check_if_entity_alive(int entity) {
    return SCENE.components[entity] != 0;
}

int check_if_entity_has_component(int entity, ComponentType type) {
    return (SCENE.components[entity] & type) == type;
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
        if (SCENE.components[entity] == 0) {
            SCENE.names[entity] = name;
            SCENE.n_entities = max(SCENE.n_entities, entity + 1);
            return entity;
        }
    }

    fprintf(stderr, "ERROR: Can't spawn more entities\n");
    exit(1);
}

int spawn_entity_copy(int entity, Transformation transformation) {
    const char* name = SCENE.names[entity];
    int entity_copy = spawn_entity(name);
    SCENE.components[entity_copy] |= TRANSFORMATION_COMPONENT;
    SCENE.transformations[entity_copy] = transformation;

    for (int i = 0; i < N_COMPONENTS; ++i) {
        ComponentType type = COMPONENT_TYPES_LIST[i];
        if (check_if_entity_has_component(entity, type)) {
            SCENE.components[entity_copy] |= type;

            switch (type) {
                case TRANSFORMATION_COMPONENT:
                    break;
                case COLLIDER_COMPONENT:
                    SCENE.colliders[entity_copy] = SCENE.colliders[entity];
                    break;
                case PRIMITIVE_COMPONENT:
                    SCENE.primitives[entity_copy]
                        = SCENE.primitives[entity];
                    break;
                case RENDER_LAYER_COMPONENT:
                    SCENE.render_layers[entity_copy]
                        = SCENE.render_layers[entity];
                    break;
                case MATERIAL_COMPONENT:
                    SCENE.materials[entity_copy] = SCENE.materials[entity];
                    break;
                case KINEMATIC_COMPONENT:
                    SCENE.kinematics[entity_copy]
                        = SCENE.kinematics[entity];
                    break;
                case VISION_COMPONENT:
                    SCENE.visions[entity_copy] = SCENE.visions[entity];
                    break;
                case RIGID_BODY_COMPONENT:
                    break;
                case OBSERVABLE_COMPONENT:
                    break;
                case TTL_COMPONENT:
                    SCENE.ttls[entity_copy] = SCENE.ttls[entity];
                    break;
                case HEALTH_COMPONENT:
                    SCENE.healths[entity_copy] = SCENE.healths[entity];
                    break;
                case GUN_COMPONENT:
                    SCENE.guns[entity_copy] = SCENE.guns[entity];
                    break;
                case BULLET_COMPONENT:
                    break;
                case OWNER_COMPONENT:
                    SCENE.owners[entity_copy] = SCENE.owners[entity];
                    break;
                default: {
                    const char* component_name = get_component_type_name(
                        type
                    );
                    fprintf(
                        stderr,
                        "ERROR: Can't copy the entity with the component: "
                        "%s\n",
                        component_name
                    );
                    exit(1);
                }
            }
        }
    }

    return entity_copy;
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
        if (SCENE.player != -1) {
            fprintf(stderr, "ERROR: Only one player can be spawned\n");
            exit(1);
        }

        SCENE.player = entity;
    }

    SCENE.transformations[entity] = transformation;
    SCENE.primitives[entity] = primitive;
    SCENE.colliders[entity] = collider;
    SCENE.materials[entity] = material;
    SCENE.render_layers[entity] = render_layer;
    SCENE.kinematics[entity] = kinematic;
    SCENE.visions[entity] = vision;
    SCENE.guns[entity] = gun;
    SCENE.healths[entity] = health;

    SCENE.components[entity] = TRANSFORMATION_COMPONENT
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
    SCENE.transformations[entity] = transformation;
    SCENE.primitives[entity] = primitive;
    SCENE.colliders[entity] = collider;
    SCENE.materials[entity] = material;
    SCENE.components[entity] = TRANSFORMATION_COMPONENT
                               | COLLIDER_COMPONENT | OBSERVABLE_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT
                               | RENDER_LAYER_COMPONENT;

    return entity;
}

int spawn_kinematic_bullet(
    Transformation transformation,
    Kinematic kinematic,
    float ttl,
    int owner
) {
    int entity = spawn_entity("Bullet");
    SCENE.transformations[entity] = transformation;
    SCENE.kinematics[entity] = kinematic;
    SCENE.ttls[entity] = ttl;
    SCENE.owners[entity] = owner;
    SCENE.components[entity] = TRANSFORMATION_COMPONENT
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
    if (SCENE.camera != -1) {
        Transformation transformation
            = SCENE.transformations[SCENE.camera];
        float aspect_ratio = (float)APP.window_width / APP.window_height;
        float height = SCENE.camera_view_width / aspect_ratio;
        Vec2 half_size = scale(vec2(SCENE.camera_view_width, height), 0.5);
        frustum.bot_left = sub(transformation.position, half_size);
        frustum.top_right = add(transformation.position, half_size);
    }

    return frustum;
}

void center_camera_on_entity(int entity) {
    Vec2 entity_position = SCENE.transformations[entity].position;
    SCENE.transformations[SCENE.camera].position = entity_position;
}

int reset_camera(void) {
    SCENE.camera = spawn_entity("Camera");
    Transformation* transformation = &SCENE.transformations[SCENE.camera];
    transformation->position = vec2(0.0, 0.0);
    transformation->orientation = 0.0;

    SCENE.components[SCENE.camera] = TRANSFORMATION_COMPONENT;
    SCENE.camera_view_width = CAMERA_VIEW_WIDTH;

    return SCENE.camera;
}

static void update_entities_scene_counter() {
    int n_entities = 0;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (SCENE.components[entity] != 0) {
            n_entities = entity + 1;
        } else if (DEBUG.picked_entity.entity == entity) {
            pick_entity(-1);
        }
    }
    SCENE.n_entities = n_entities;
    DEBUG.general.n_entities = n_entities;
}

void update_scene(float dt) {
    update_visions();
    update_camera();

    if (DEBUG.is_playing) {
        update_ttls(dt);
        update_healths();
        update_player();
        update_bullets(dt);
        update_kinematics(dt);
        update_entities_scene_counter();
    } else {
        update_entity_picking();
        update_entity_dragging();
    }

    update_collisions();
}
