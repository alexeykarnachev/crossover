#pragma once
#include "component.h"
#include "math.h"
#include "nfd.h"

typedef struct Project {
    int version;
    const char* project_file_path;
    const char* scene_file_path;
    const char* default_search_path;
} Project;

typedef struct PickedEntity {
    ComponentType component_type;
    int entity;
    int is_dragging;
    int dragging_handle_idx;
} PickedEntity;

typedef struct Editor {
    Project project;
    PickedEntity picked_entity;
    int is_playing;
    int entity_to_copy;
} Editor;

extern Editor EDITOR;

Vec2 get_cursor_scene_pos(void);

void init_editor(void);

void create_project_via_nfd(const nfdchar_t* search_path);
void load_project_via_nfd(const nfdchar_t* search_path);
const char* load_scene_via_nfd(const char* search_path);
const char* save_scene_via_nfd(const char* search_path);

void new_editor_project(void);
void new_editor_scene(void);
void open_editor_project(void);
void open_editor_scene(void);
void save_editor_scene(void);
void save_editor_scene_as(void);

int load_editor_project(const char* file_path);
int save_editor_project(void);

void pick_entity(int entity);
int check_if_cursor_on_entity(int entity);
int get_entity_under_cursor(void);

void update_editor(void);
void update_editor_gui(void);
void update_entity_picking(void);
void update_entity_dragging(void);

void render_editor(void);
void render_entity_handles(void);
