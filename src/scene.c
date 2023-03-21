#include "scene.h"

#include "app.h"
#include "asset.h"
#include "collision.h"
#include "component.h"
#include "const.h"
#include "debug.h"
#include "editor.h"
#include "math.h"
#include "nfd.h"
#include "system.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Scene SCENE = {0};

Vec2 get_cursor_scene_pos(void) {
    Vec2 screen_pos = get_cursor_screen_pos();
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;

    Transformation camera = SCENE.transformations[SCENE.camera];
    Vec2 position = rotate(
        vec2(x, y), vec2(0.0, 0.0), camera.curr_orientation
    );
    return position;
}

void reset_scene(void) {
    memset(SCENE.components, 0, sizeof(uint64_t) * MAX_N_ENTITIES);
    memset(SCENE.owners, -1, sizeof(int) * MAX_N_ENTITIES);
    memset(SCENE.scorers, 0, sizeof(Scorer) * MAX_N_ENTITIES);
    memset(SCENE.hiddens, 0, sizeof(uint64_t) * MAX_N_ENTITIES);

    for (int e = 0; e < MAX_N_ENTITIES; ++e) {
        destroy_array(&SCENE.entity_to_tiles[e]);
        SCENE.entity_to_tiles[e] = init_array();
    }

    for (int t = 0; t < N_SCENE_TILES; ++t) {
        destroy_array(&SCENE.tile_to_entities[t]);
        SCENE.tile_to_entities[t] = init_array();
    }

    destroy_assets();

    reset_camera();

    SCENE.n_entities = 0;
}

void save_scene(const char* file_path, ResultMessage* res_msg) {
    FILE* fp = open_file(file_path, res_msg, "wb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return;
    }

    int version = SCENE_VERSION;

    fwrite(&version, sizeof(int), 1, fp);

    fwrite(&SCENE.time, sizeof(float), 1, fp);
    fwrite(&SCENE.n_entities, sizeof(int), 1, fp);
    fwrite(SCENE.components, sizeof(uint64_t), SCENE.n_entities, fp);
    fwrite(SCENE.names, sizeof(SCENE.names), SCENE.n_entities, fp);
    fwrite(
        SCENE.transformations, sizeof(Transformation), SCENE.n_entities, fp
    );
    fwrite(SCENE.rigid_bodies, sizeof(RigidBody), SCENE.n_entities, fp);
    fwrite(SCENE.visions, sizeof(Vision), SCENE.n_entities, fp);
    fwrite(SCENE.colliders, sizeof(Primitive), SCENE.n_entities, fp);
    fwrite(SCENE.primitives, sizeof(Primitive), SCENE.n_entities, fp);
    fwrite(SCENE.materials, sizeof(Material), SCENE.n_entities, fp);
    fwrite(SCENE.guns, sizeof(Gun), SCENE.n_entities, fp);
    fwrite(SCENE.bullets, sizeof(Bullet), SCENE.n_entities, fp);
    fwrite(SCENE.ttls, sizeof(float), SCENE.n_entities, fp);
    fwrite(SCENE.healths, sizeof(Health), SCENE.n_entities, fp);
    fwrite(SCENE.render_layers, sizeof(float), SCENE.n_entities, fp);
    fwrite(SCENE.owners, sizeof(int), SCENE.n_entities, fp);
    fwrite(SCENE.controllers, sizeof(Controller), SCENE.n_entities, fp);
    write_n_scorers(fp, SCENE.scorers, SCENE.n_entities);
    fwrite(SCENE.hiddens, sizeof(uint64_t), SCENE.n_entities, fp);
    fwrite(&SCENE.camera, sizeof(int), 1, fp);
    fwrite(&SCENE.camera_view_width, sizeof(float), 1, fp);

    // Write Brains
    fwrite(&N_BRAINS, sizeof(int), 1, fp);
    int n_brains = 0;
    for (int i = 0; i < BRAINS_ARRAY_CAPACITY; ++i) {
        Brain* brain = &BRAINS[i];
        if (strlen(brain->params.key) == 0) {
            continue;
        }

        write_str_to_file(brain->params.key, fp, 0);
        n_brains += 1;
    }

    if (n_brains != N_BRAINS) {
        fprintf(
            stderr,
            "ERROR: Number of saved Brains (%d) is not equal to the "
            "number of "
            "Brains registered in the engine runtime (%d). It's a bug\n",
            n_brains,
            N_BRAINS
        );
        exit(1);
    }

    // Write debug info
    fwrite(&DEBUG, sizeof(DEBUG), 1, fp);

    // Close the file
    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Scene is saved");
    return;
}

void load_scene(const char* file_path, ResultMessage* res_msg) {
    FILE* fp = open_file(file_path, res_msg, "rb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return;
    }

    reset_scene();
    destroy_assets();

    // TODO: Also, check some kind of hash which should be saved as a
    // Scene property. It will prevent scene from modifications through
    // the file
    int version;
    fread(&version, sizeof(int), 1, fp);
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

    fread(&SCENE.time, sizeof(float), 1, fp);
    fread(&SCENE.n_entities, sizeof(int), 1, fp);

    memset(&SCENE.need_update_tiling, 1, sizeof(int) * MAX_N_ENTITIES);

    fread(SCENE.components, sizeof(uint64_t), SCENE.n_entities, fp);
    fread(SCENE.names, sizeof(SCENE.names), SCENE.n_entities, fp);
    fread(
        SCENE.transformations, sizeof(Transformation), SCENE.n_entities, fp
    );
    fread(SCENE.rigid_bodies, sizeof(RigidBody), SCENE.n_entities, fp);
    fread(SCENE.visions, sizeof(Vision), SCENE.n_entities, fp);
    fread(SCENE.colliders, sizeof(Primitive), SCENE.n_entities, fp);
    fread(SCENE.primitives, sizeof(Primitive), SCENE.n_entities, fp);
    fread(SCENE.materials, sizeof(Material), SCENE.n_entities, fp);
    fread(SCENE.guns, sizeof(Gun), SCENE.n_entities, fp);
    fread(SCENE.bullets, sizeof(Bullet), SCENE.n_entities, fp);
    fread(SCENE.ttls, sizeof(float), SCENE.n_entities, fp);
    fread(SCENE.healths, sizeof(Health), SCENE.n_entities, fp);
    fread(SCENE.render_layers, sizeof(float), SCENE.n_entities, fp);
    fread(SCENE.owners, sizeof(int), SCENE.n_entities, fp);
    fread(SCENE.controllers, sizeof(Controller), SCENE.n_entities, fp);
    read_n_scorers(fp, SCENE.scorers, SCENE.n_entities);
    fread(SCENE.scorers, sizeof(uint64_t), SCENE.n_entities, fp);
    fread(&SCENE.camera, sizeof(int), 1, fp);
    fread(&SCENE.camera_view_width, sizeof(float), 1, fp);

    // Read Brains
    int n_brains;
    fread(&n_brains, sizeof(int), 1, fp);
    for (int i = 0; i < n_brains; ++i) {
        char* key;
        read_str_from_file(&key, fp, 0);
        Brain* brain = load_brain(key, res_msg, 0);
        free(key);
        // TODO: Currently `get_brain_size` returns the number weights.
        // It may be confusing. It's better to make the explicitly named
        // function `get_brain_n_weights` as well as `get_brain_size`
        // which returns the real total size in bytes
    }

    // Read debug info
    fread(&DEBUG, sizeof(DEBUG), 1, fp);

    // Close the file
    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Scene is loaded");
    return;
}

void hide_entity(int entity) {
    if (check_if_entity_hidden(entity) == 1) {
        fprintf(stderr, "ERROR: Can't hide an already hidden entity\n");
        exit(1);
    }
    SCENE.hiddens[entity] = SCENE.components[entity];
    SCENE.components[entity] = HIDDEN_COMPONENT;
}

void reveal_entity(int entity) {
    if (check_if_entity_hidden(entity) == 0) {
        fprintf(stderr, "ERROR: Can't reveal a not hidden entity\n");
        exit(1);
    }
    SCENE.components[entity] = SCENE.hiddens[entity];
    SCENE.hiddens[entity] = 0;
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

    SCENE.healths[entity] = init_default_health();
    memset(&SCENE.scorers[entity], 0, sizeof(Scorer));
    SCENE.hiddens[entity] = 0;
    SCENE.names[entity][0] = '\0';
    entity_leaves_all_tiles(entity);
}

void entity_enters_tile(int entity, int tile) {
    array_push(&SCENE.tile_to_entities[tile], entity);
    array_push(&SCENE.entity_to_tiles[entity], tile);
}

void entity_leaves_all_tiles(int entity) {
    Array* tiles = &SCENE.entity_to_tiles[entity];
    for (int t = 0; t < tiles->length; ++t) {
        int tile = (int)array_get(tiles, t);
        array_remove_value(&SCENE.tile_to_entities[tile], entity, 0);
    }
    empty_array(tiles);
}

Vec2 get_tile_location_at(Vec2 position) {
    float total_width = N_X_SCENE_TILES * SCENE_TILE_SIZE;
    float total_height = N_Y_SCENE_TILES * SCENE_TILE_SIZE;
    float right_x = 0.5 * total_width;
    float top_y = 0.5 * total_height;
    float left_x = -right_x;
    float bot_y = -top_y;

    float x = position.x;
    float y = position.y;
    int col = floor((x - left_x) / SCENE_TILE_SIZE);
    int row = floor((top_y - y) / SCENE_TILE_SIZE);
    col = min(max(0, col), N_X_SCENE_TILES);
    row = min(max(0, row), N_Y_SCENE_TILES);
    return vec2(col, row);
}

int check_if_entity_hidden(int entity) {
    int is_hidden = SCENE.hiddens[entity] != 0;
    int only_hidden = SCENE.hiddens[entity] == HIDDEN_COMPONENT;
    if (is_hidden && only_hidden) {
        fprintf(
            stderr,
            "ERROR: Can't determine if the entity is hidden. It has "
            "hidden component active but also it has another components "
            "active. It's a bug\n"
        );
        exit(1);
    }
    return is_hidden;
}

int check_if_entity_alive(int entity) {
    return SCENE.components[entity] != 0;
}

int check_if_entity_has_component(int entity, ComponentType type) {
    return (SCENE.components[entity] & type) == type;
}

int check_if_entity_has_hidden_component(int entity, ComponentType type) {
    return (SCENE.hiddens[entity] & type) == type;
}

static int spawn_entity(char* name) {
    if (strlen(name) > MAX_ENTITY_NAME_LENGTH) {
        fprintf(
            stderr,
            "ERROR: Max. entity name can't be larget than %d\n",
            MAX_ENTITY_NAME_LENGTH
        );
        exit(1);
    }

    for (int entity = 0; entity < MAX_N_ENTITIES; ++entity) {
        if (SCENE.components[entity] == 0) {
            strcpy(SCENE.names[entity], name);
            SCENE.n_entities = max(SCENE.n_entities, entity + 1);
            return entity;
        }
    }

    fprintf(stderr, "ERROR: Can't spawn more entities\n");
    exit(1);
}

int spawn_entity_copy(int entity, Transformation transformation) {
    int entity_copy = spawn_entity(SCENE.names[entity]);
    SCENE.components[entity_copy] |= TRANSFORMATION_COMPONENT;
    SCENE.transformations[entity_copy] = transformation;

    for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
        ComponentType type = COMPONENT_TYPES[i];
        if (check_if_entity_has_component(entity, type)) {
            SCENE.components[entity_copy] |= type;

            switch (type) {
                case TRANSFORMATION_COMPONENT:
                    update_orientation(
                        entity_copy,
                        SCENE.transformations[entity].curr_orientation
                    );
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
                    SCENE.bullets[entity_copy] = SCENE.bullets[entity];
                    break;
                case OWNER_COMPONENT:
                    SCENE.owners[entity_copy] = SCENE.owners[entity];
                    break;
                case CONTROLLER_COMPONENT:
                    SCENE.controllers[entity_copy]
                        = SCENE.controllers[entity];
                    break;
                case SCORER_COMPONENT:
                    SCENE.scorers[entity_copy] = SCENE.scorers[entity];
                    break;
                case HIDDEN_COMPONENT:
                    SCENE.hiddens[entity_copy] = SCENE.hiddens[entity];
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
    Vision vision,
    Gun gun,
    Controller controller,
    Health health
) {
    int entity = spawn_entity("Guy");

    SCENE.transformations[entity] = transformation;
    SCENE.rigid_bodies[entity] = rigid_body;
    SCENE.primitives[entity] = primitive;
    SCENE.colliders[entity] = collider;
    SCENE.materials[entity] = material;
    SCENE.render_layers[entity] = render_layer;
    SCENE.visions[entity] = vision;
    SCENE.guns[entity] = gun;
    SCENE.healths[entity] = health;
    SCENE.controllers[entity] = controller;

    SCENE.components[entity] = TRANSFORMATION_COMPONENT | VISION_COMPONENT
                               | OBSERVABLE_COMPONENT | COLLIDER_COMPONENT
                               | RIGID_BODY_COMPONENT | PRIMITIVE_COMPONENT
                               | MATERIAL_COMPONENT | GUN_COMPONENT
                               | HEALTH_COMPONENT | CONTROLLER_COMPONENT
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

int spawn_sprite(
    Transformation transformation,
    Primitive primitive,
    Material material,
    float render_layer
) {
    int entity = spawn_entity("Obstacle");
    SCENE.transformations[entity] = transformation;
    SCENE.primitives[entity] = primitive;
    SCENE.materials[entity] = material;
    SCENE.components[entity] = TRANSFORMATION_COMPONENT
                               | PRIMITIVE_COMPONENT | MATERIAL_COMPONENT
                               | RENDER_LAYER_COMPONENT;

    return entity;
}

int spawn_bullet(Bullet bullet, int owner, float ttl) {
    int entity = spawn_entity("Bullet");
    SCENE.transformations[entity] = SCENE.transformations[owner];
    SCENE.bullets[entity] = bullet;
    SCENE.ttls[entity] = ttl;
    SCENE.owners[entity] = owner;
    SCENE.components[entity] = TRANSFORMATION_COMPONENT | TTL_COMPONENT
                               | BULLET_COMPONENT | OWNER_COMPONENT;
}

int spawn_default_ai_guy(
    Transformation transformation, Controller controller
) {
    return spawn_guy(
        transformation,
        init_default_dynamic_rigid_body(),
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_color_material(REDWOOD_COLOR),
        0.0,
        init_vision(0.5 * PI, 30.0, 32),
        init_gun(4.0, 100.0, 5.0),
        controller,
        init_default_health()
    );
}

int spawn_default_dummy_ai_guy(Transformation transformation) {
    Controller controller = init_default_dummy_ai_controller();
    return spawn_default_ai_guy(transformation, controller);
}

int spawn_default_brain_ai_guy(Transformation transformation) {
    Controller controller = init_default_brain_ai_controller();
    return spawn_default_ai_guy(transformation, controller);
}

int spawn_default_player_keyboard_guy(Transformation transformation) {
    return spawn_guy(
        transformation,
        init_default_dynamic_rigid_body(),
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_color_material(FOREST_GREEN_COLOR),
        0.0,
        init_vision(0.5 * PI, 10.0, 32),
        init_gun(4.0, 100.0, 5.0),
        init_default_player_keyboard_controller(),
        init_default_health()
    );
}

int spawn_default_circle_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
        init_default_circle_primitive(),
        init_default_circle_primitive(),
        init_color_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_rectangle_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
        init_default_rectangle_primitive(),
        init_default_rectangle_primitive(),
        init_default_wall_material(),
        0.0
    );
}

int spawn_default_line_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
        init_default_line_primitive(),
        init_default_line_primitive(),
        init_color_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_polygon_obstacle(Transformation transformation) {
    return spawn_obstacle(
        transformation,
        init_default_static_rigid_body(),
        init_default_polygon_primitive(),
        init_default_polygon_primitive(),
        init_color_material(GRAY_COLOR),
        0.0
    );
}

int spawn_default_circle_sprite(Transformation transformation) {
    return spawn_sprite(
        transformation,
        init_default_circle_primitive(),
        init_default_wall_material(),
        0.0
    );
}

int spawn_default_rectangle_sprite(Transformation transformation) {
    return spawn_sprite(
        transformation,
        init_default_rectangle_primitive(),
        init_default_wall_material(),
        0.0
    );
}

int spawn_default_line_sprite(Transformation transformation) {
    return spawn_sprite(
        transformation,
        init_default_line_primitive(),
        init_default_wall_material(),
        0.0
    );
}

int spawn_default_polygon_sprite(Transformation transformation) {
    return spawn_sprite(
        transformation,
        init_default_polygon_primitive(),
        init_default_wall_material(),
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
        frustum.bot_left = sub(transformation.curr_position, half_size);
        frustum.top_right = add(transformation.curr_position, half_size);
    }

    return frustum;
}

void center_camera_on_entity(int entity) {
    Vec2 entity_position = SCENE.transformations[entity].curr_position;
    update_position(SCENE.camera, entity_position);
}

int reset_camera(void) {
    SCENE.camera = spawn_entity("Camera");
    Transformation* transformation = &SCENE.transformations[SCENE.camera];
    update_position(SCENE.camera, vec2(0.0, 0.0));
    update_orientation(SCENE.camera, 0.0);

    SCENE.components[SCENE.camera] = TRANSFORMATION_COMPONENT;
    SCENE.camera_view_width = CAMERA_VIEW_WIDTH;

    return SCENE.camera;
}

void update_position(int entity, Vec2 position) {
    Transformation* t = &SCENE.transformations[entity];
    t->prev_position = t->curr_position;
    t->curr_position = position;
    SCENE.need_update_tiling[entity] = 1;
}

void update_orientation(int entity, float orientation) {
    orientation = fmodf(orientation, 2.0 * PI);
    Transformation* t = &SCENE.transformations[entity];
    t->prev_orientation = t->curr_orientation;
    t->curr_orientation = orientation;
    SCENE.need_update_tiling[entity] = 1;
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
    profiler_push(PROFILER, "update_visions");
    update_visions();
    profiler_pop(PROFILER);

    update_camera();

    if (is_playing) {
        SCENE.time += dt;

        update_ttls(dt);
        update_healths(dt);

        profiler_push(PROFILER, "update_controllers");
        update_controllers();
        profiler_pop(PROFILER);

        profiler_push(PROFILER, "update_bullets");
        update_bullets(dt);
        profiler_pop(PROFILER);

        profiler_push(PROFILER, "update_rigid_bodies");
        update_rigid_bodies(dt);
        profiler_pop(PROFILER);

        profiler_push(PROFILER, "update_entities_scene_counter");
        update_entities_scene_counter();
        profiler_pop(PROFILER);
    }

    profiler_push(PROFILER, "update_collisions");
    for (int i = 0; i < DEBUG.collisions.n_update_steps; ++i) {
        update_collisions(is_playing);
    }
    profiler_pop(PROFILER);
}
