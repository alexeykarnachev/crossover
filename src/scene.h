#pragma once
#include "array.h"
#include "asset.h"
#include "component.h"
#include "const.h"
#include "math.h"
#include "system.h"
#include <stdint.h>

#define N_SCENE_TILES N_X_SCENE_TILES* N_Y_SCENE_TILES

typedef struct Scene {
    float time;
    int n_entities;

    // Scene tiles
    int need_update_tiling[MAX_N_ENTITIES];
    Array entity_to_tiles[MAX_N_ENTITIES];
    Array tile_to_entities[N_SCENE_TILES];

    // Components
    uint64_t components[MAX_N_ENTITIES];
    char names[MAX_N_ENTITIES][MAX_ENTITY_NAME_LENGTH];

    Transformation transformations[MAX_N_ENTITIES];
    RigidBody rigid_bodies[MAX_N_ENTITIES];
    Vision visions[MAX_N_ENTITIES];
    Primitive colliders[MAX_N_ENTITIES];
    Primitive primitives[MAX_N_ENTITIES];
    Material materials[MAX_N_ENTITIES];
    Gun guns[MAX_N_ENTITIES];
    Bullet bullets[MAX_N_ENTITIES];
    float ttls[MAX_N_ENTITIES];
    Health healths[MAX_N_ENTITIES];
    float render_layers[MAX_N_ENTITIES];
    int owners[MAX_N_ENTITIES];
    Controller controllers[MAX_N_ENTITIES];
    Scorer scorers[MAX_N_ENTITIES];
    uint64_t hiddens[MAX_N_ENTITIES];

    // Singleton entities
    int camera;
    float camera_view_width;
} Scene;

extern Scene SCENE;

typedef struct CameraFrustum {
    Vec2 bot_left;
    Vec2 top_right;
} CameraFrustum;

CameraFrustum get_camera_frustum();
void center_camera_on_entity(int entity);
int reset_camera(void);

Vec2 get_cursor_scene_pos(void);

void reset_scene(void);
void save_scene(const char* file_path, ResultMessage* res_msg);
void load_scene(const char* file_path, ResultMessage* res_msg);
void update_scene(float dt, int is_playing);
void update_position(int entity, Vec2 position);
void update_orientation(int entity, float orientation);

void entity_enters_tile(int entity, int tile);
void entity_leaves_tile(int entity, int tile);
void entity_leaves_all_tiles(int entity);
Vec2 get_tile_location_at(Vec2 position);

int check_if_entity_hidden(int entity);
int check_if_entity_alive(int entity);
int check_if_entity_has_component(int entity, ComponentType component);

void hide_entity(int entity);
void reveal_entity(int entity);
void destroy_entity(int entity);
int spawn_entity_copy(int entity, Transformation transformation);
int spawn_camera(Transformation transformation);
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
);
int spawn_obstacle(
    Transformation transformation,
    RigidBody rigid_body,
    Primitive primitive,
    Primitive collider,
    Material material,
    float render_layer
);
int spawn_sprite(
    Transformation transformation,
    Primitive primitive,
    Material material,
    float render_layer
);
int spawn_bullet(Bullet bullet, int owner, float ttl);
int spawn_default_player_keyboard_guy(Transformation transformation);
int spawn_default_dummy_ai_guy(Transformation transformation);
int spawn_default_brain_ai_guy(Transformation transformation);
int spawn_default_circle_obstacle(Transformation transformation);
int spawn_default_rectangle_obstacle(Transformation transformation);
int spawn_default_line_obstacle(Transformation transformation);
int spawn_default_polygon_obstacle(Transformation transformation);
int spawn_default_circle_sprite(Transformation transformation);
int spawn_default_rectangle_sprite(Transformation transformation);
int spawn_default_line_sprite(Transformation transformation);
int spawn_default_polygon_sprite(Transformation transformation);
