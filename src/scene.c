#include "scene.h"

#include "app.h"
#include "brain.h"
#include "component.h"
#include "const.h"
#include "debug.h"
#include "math.h"
#include "nfd.h"
#include "system.h"
#include "utils.h"
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
    SCENE.n_entities = 0;
    memset(SCENE.components, 0, sizeof(uint64_t) * MAX_N_ENTITIES);
    reset_camera();
}

void save_scene(const char* file_path, ResultMessage* res_msg) {
    memset(res_msg, 0, sizeof(ResultMessage));

    if (file_path == NULL) {
        strcpy(res_msg->msg, "ERROR: Can't save the Scene to a NULL file");
        return;
    }

    FILE* fp = fopen(file_path, "wb");
    if (!fp) {
        strcpy(
            res_msg->msg, "ERROR: Can't open the file to save the Scene"
        );
        return;
    }

    int version = SCENE_VERSION;
    int n_bytes = 0;
    n_bytes += fwrite(&version, sizeof(int), 1, fp);
    n_bytes += fwrite(&SCENE.time, sizeof(float), 1, fp);
    n_bytes += fwrite(&SCENE.n_entities, sizeof(int), 1, fp);
    n_bytes += fwrite(
        SCENE.components, sizeof(uint64_t), SCENE.n_entities, fp
    );

    for (int i = 0; i < SCENE.n_entities; ++i) {
        n_bytes += write_str_to_file(SCENE.names[i], fp, 0);
    }

    n_bytes += fwrite(
        SCENE.transformations, sizeof(Transformation), SCENE.n_entities, fp
    );
    n_bytes += fwrite(
        SCENE.rigid_bodies, sizeof(RigidBody), SCENE.n_entities, fp
    );
    n_bytes += fwrite(
        SCENE.kinematic_movements,
        sizeof(KinematicMovement),
        SCENE.n_entities,
        fp
    );
    n_bytes += fwrite(SCENE.visions, sizeof(Vision), SCENE.n_entities, fp);
    n_bytes += fwrite(
        SCENE.colliders, sizeof(Primitive), SCENE.n_entities, fp
    );
    n_bytes += fwrite(
        SCENE.primitives, sizeof(Primitive), SCENE.n_entities, fp
    );
    n_bytes += fwrite(
        SCENE.materials, sizeof(Material), SCENE.n_entities, fp
    );
    n_bytes += fwrite(SCENE.guns, sizeof(Gun), SCENE.n_entities, fp);
    n_bytes += fwrite(SCENE.ttls, sizeof(float), SCENE.n_entities, fp);
    n_bytes += fwrite(SCENE.healths, sizeof(float), SCENE.n_entities, fp);
    n_bytes += fwrite(
        SCENE.render_layers, sizeof(float), SCENE.n_entities, fp
    );
    n_bytes += fwrite(SCENE.owners, sizeof(int), SCENE.n_entities, fp);
    n_bytes += fwrite(
        SCENE.controllers, sizeof(Controller), SCENE.n_entities, fp
    );
    n_bytes += fwrite(&SCENE.camera, sizeof(int), 1, fp);
    n_bytes += fwrite(&SCENE.camera_view_width, sizeof(float), 1, fp);

    fclose(fp);
    res_msg->flag = 1;

    sprintf(res_msg->msg, "INFO: Scene is saved (%dB)", n_bytes);
    return;
}

void load_scene(const char* file_path, ResultMessage* res_msg) {
    memset(res_msg, 0, sizeof(ResultMessage));

    if (file_path == NULL) {
        strcpy(
            res_msg->msg, "ERROR: Can't load the Scene from a NULL file"
        );
        return;
    }

    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        strcpy(
            res_msg->msg, "ERROR: Can't open the file to load the Scene"
        );
        return;
    }

    int version;
    int n_bytes = 0;
    n_bytes += fread(&version, sizeof(int), 1, fp);
    if (version != SCENE_VERSION) {
        sprintf(
            res_msg->msg,
            "ERROR: Scene version %d is not compatible with the engine, "
            "expecting the version %d\n",
            version,
            SCENE_VERSION
        );
        return;
    }

    n_bytes += fread(&SCENE.time, sizeof(float), 1, fp);
    n_bytes += fread(&SCENE.n_entities, sizeof(int), 1, fp);
    n_bytes += fread(
        SCENE.components, sizeof(uint64_t), SCENE.n_entities, fp
    );

    for (int i = 0; i < SCENE.n_entities; ++i) {
        uint32_t name_len;
        n_bytes += fread(&name_len, sizeof(uint32_t), 1, fp);

        int name_size = name_len + 1;
        char* buffer = (char*)malloc(name_size);
        n_bytes += name_size;
        n_bytes += fread(buffer, sizeof(char), name_len, fp);
        buffer[name_len] = '\0';

        SCENE.names[i] = buffer;
    }

    n_bytes += fread(
        SCENE.transformations, sizeof(Transformation), SCENE.n_entities, fp
    );
    n_bytes += fread(
        SCENE.rigid_bodies, sizeof(RigidBody), SCENE.n_entities, fp
    );
    n_bytes += fread(
        SCENE.kinematic_movements,
        sizeof(KinematicMovement),
        SCENE.n_entities,
        fp
    );
    n_bytes += fread(SCENE.visions, sizeof(Vision), SCENE.n_entities, fp);
    n_bytes += fread(
        SCENE.colliders, sizeof(Primitive), SCENE.n_entities, fp
    );
    n_bytes += fread(
        SCENE.primitives, sizeof(Primitive), SCENE.n_entities, fp
    );
    n_bytes += fread(
        SCENE.materials, sizeof(Material), SCENE.n_entities, fp
    );
    n_bytes += fread(SCENE.guns, sizeof(Gun), SCENE.n_entities, fp);
    n_bytes += fread(SCENE.ttls, sizeof(float), SCENE.n_entities, fp);
    n_bytes += fread(SCENE.healths, sizeof(float), SCENE.n_entities, fp);
    n_bytes += fread(
        SCENE.render_layers, sizeof(float), SCENE.n_entities, fp
    );
    n_bytes += fread(SCENE.owners, sizeof(int), SCENE.n_entities, fp);
    n_bytes += fread(
        SCENE.controllers, sizeof(Controller), SCENE.n_entities, fp
    );
    n_bytes += fread(&SCENE.camera, sizeof(int), 1, fp);
    n_bytes += fread(&SCENE.camera_view_width, sizeof(float), 1, fp);

    fclose(fp);
    res_msg->flag = 1;

    sprintf(res_msg->msg, "INFO: Scene is loaded (%dB)", n_bytes);
    return;
}

void destroy_entity(int entity) {
    SCENE.components[entity] = 0;

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

    for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
        ComponentType type = COMPONENT_TYPES[i];
        if (check_if_entity_has_component(entity, type)) {
            SCENE.components[entity_copy] |= type;

            switch (type) {
                case TRANSFORMATION_COMPONENT:
                    break;
                case RIGID_BODY_COMPONENT:
                    SCENE.rigid_bodies[entity_copy]
                        = SCENE.rigid_bodies[entity];
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
                case KINEMATIC_MOVEMENT_COMPONENT:
                    SCENE.kinematic_movements[entity_copy]
                        = SCENE.kinematic_movements[entity];
                    break;
                case VISION_COMPONENT:
                    SCENE.visions[entity_copy] = SCENE.visions[entity];
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
                case CONTROLLER_COMPONENT:
                    SCENE.controllers[entity_copy]
                        = SCENE.controllers[entity];
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

int spawn_guy(
    Transformation transformation,
    RigidBody rigid_body,
    Primitive primitive,
    Primitive collider,
    Material material,
    float render_layer,
    KinematicMovement kinematic_movement,
    Vision vision,
    Gun gun,
    Controller controller,
    float health
) {
    int entity = spawn_entity("Guy");

    SCENE.transformations[entity] = transformation;
    SCENE.rigid_bodies[entity] = rigid_body;
    SCENE.primitives[entity] = primitive;
    SCENE.colliders[entity] = collider;
    SCENE.materials[entity] = material;
    SCENE.render_layers[entity] = render_layer;
    SCENE.kinematic_movements[entity] = kinematic_movement;
    SCENE.visions[entity] = vision;
    SCENE.guns[entity] = gun;
    SCENE.healths[entity] = health;
    SCENE.controllers[entity] = controller;

    SCENE.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_MOVEMENT_COMPONENT
                               | VISION_COMPONENT | OBSERVABLE_COMPONENT
                               | COLLIDER_COMPONENT | RIGID_BODY_COMPONENT
                               | PRIMITIVE_COMPONENT | MATERIAL_COMPONENT
                               | GUN_COMPONENT | HEALTH_COMPONENT
                               | CONTROLLER_COMPONENT
                               | RENDER_LAYER_COMPONENT;

    return entity;
}

int spawn_obstacle(
    Transformation transformation,
    RigidBody rigid_body,
    Primitive primitive,
    Primitive collider,
    Material material,
    float render_layer
) {
    int entity = spawn_entity("Obstacle");
    SCENE.transformations[entity] = transformation;
    SCENE.rigid_bodies[entity] = rigid_body;
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

int spawn_bullet(
    Transformation transformation,
    KinematicMovement kinematic_movement,
    float ttl,
    int owner
) {
    int entity = spawn_entity("Bullet");
    SCENE.transformations[entity] = transformation;
    SCENE.kinematic_movements[entity] = kinematic_movement;
    SCENE.ttls[entity] = ttl;
    SCENE.owners[entity] = owner;
    SCENE.components[entity] = TRANSFORMATION_COMPONENT
                               | KINEMATIC_MOVEMENT_COMPONENT
                               | TTL_COMPONENT | BULLET_COMPONENT
                               | OWNER_COMPONENT;
}

int spawn_default_ai_guy(
    Transformation transformation, Controller controller
) {
    return spawn_guy(
        transformation,
        init_default_kinematic_rigid_body(),
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_material(REDWOOD_COLOR),
        0.0,
        init_kinematic_movement(5.0, 0.0, 0.0, 0),
        init_vision(0.5 * PI, 10.0, 32),
        init_gun(4.0, 100.0, 5.0),
        controller,
        1000.0
    );
}

int spawn_default_dummy_ai_guy(Transformation transformation) {
    Controller controller = init_dummy_ai_controller();
    return spawn_default_ai_guy(transformation, controller);
}

int spawn_default_brain_ai_guy(Transformation transformation) {
    Controller controller = init_brain_ai_controller();
    return spawn_default_ai_guy(transformation, controller);
}

int spawn_default_player_keyboard_guy(Transformation transformation) {
    return spawn_guy(
        transformation,
        init_default_kinematic_rigid_body(),
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_material(FOREST_GREEN_COLOR),
        0.0,
        init_kinematic_movement(5.0, 0.0, 0.0, 0),
        init_vision(0.5 * PI, 10.0, 32),
        init_gun(4.0, 100.0, 5.0),
        init_player_keyboard_controller(),
        1000.0
    );
}

int spawn_default_circle_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_rectangle_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
        init_default_rectangle_primitive(),
        init_default_rectangle_primitive(),
        init_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_line_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
        init_default_line_primitive(),
        init_default_line_primitive(),
        init_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_polygon_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
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
        }
    }
    SCENE.n_entities = n_entities;
}

void update_scene(float dt, int is_playing) {
    update_visions();
    update_camera();

    if (is_playing) {
        SCENE.time += dt;
        update_ttls(dt);
        update_healths();
        update_controllers();
        update_bullets(dt);
        update_kinematic_movements(dt);
        update_entities_scene_counter();
    }

    update_collisions();
}
