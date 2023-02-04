#pragma once
#include "component.h"
#include "const.h"
#include "system.h"
#include <stdint.h>

typedef struct Scene {
    int version;
    int n_entities;

    // Components
    uint64_t components[MAX_N_ENTITIES];
    const char* names[MAX_N_ENTITIES];
    Transformation transformations[MAX_N_ENTITIES];
    Kinematic kinematics[MAX_N_ENTITIES];
    Vision visions[MAX_N_ENTITIES];
    Primitive colliders[MAX_N_ENTITIES];
    Primitive primitives[MAX_N_ENTITIES];
    Material materials[MAX_N_ENTITIES];
    Gun guns[MAX_N_ENTITIES];
    float ttls[MAX_N_ENTITIES];
    float healths[MAX_N_ENTITIES];
    float render_layers[MAX_N_ENTITIES];
    int owners[MAX_N_ENTITIES];

    // Singleton entities
    int camera;
    int player;

    // Other configuration
    float camera_view_width;
} Scene;

extern Scene SCENE;

typedef struct CameraFrustum {
    Vec2 bot_left;
    Vec2 top_right;
} CameraFrustum;

CameraFrustum get_camera_frustum();
void center_camera_on_entity(int entity);
Vec2 get_cursor_scene_pos(void);
void init_scene(void);
int save_scene(const char* file_path);
int load_scene(const char* file_path);
void destroy_entity(int entity);
int check_if_entity_alive(int entity);
int check_if_entity_has_component(int entity, ComponentType component);

int spawn_entity_copy(int entity, Transformation transformation);
int spawn_camera(Transformation transformation);
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
);
int spawn_renderable_obstacle(
    Transformation transformation,
    Primitive primitive,
    Primitive collider,
    Material material,
    float render_layer
);
int spawn_kinematic_bullet(
    Transformation transformation,
    Kinematic kinematic,
    float ttl,
    int owner
);
int spawn_default_renderable_guy(Transformation transformation);
int spawn_default_renderable_circle_obstacle(Transformation transformation
);
int spawn_default_renderable_rectangle_obstacle(
    Transformation transformation
);
int spawn_default_renderable_line_obstacle(Transformation transformation);
int spawn_default_renderable_polygon_obstacle(Transformation transformation
);
void update_scene(float dt);
