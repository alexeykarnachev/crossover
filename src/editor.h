#pragma once
#include "component.h"
#include "math.h"
#include "nfd.h"

typedef struct Project {
    char* project_file_path;
    char* scene_file_path;
    char* default_search_path;
} Project;

typedef struct PickedEntity {
    ComponentType component_type;
    int entity;
    int is_dragging;
    int dragging_handle_idx;
} PickedEntity;

typedef struct Editor {
    struct {
        int ctrl;
        int del;
        int n;
        int o;
        int s;
        int q;
        int c;
        int v;
    } key;

    Project project;
    PickedEntity picked_entity;
    Transformation cursor_transformation;

    int can_copy;
    int can_paste;
    int can_delete;

    int is_playing;
    int entity_to_copy;
    int is_editing_brain;
    int is_editing_simulation;
} Editor;

extern Editor EDITOR;

Vec2 get_cursor_scene_pos(void);

void init_editor(void);
void reset_editor(void);

void new_editor_project(void);
void new_editor_scene(void);
void open_editor_project(void);
void open_editor_scene(void);
void save_editor_scene(void);
void save_editor_scene_as(void);

void load_editor_project(const char* file_path, ResultMessage* res_msg);
void save_editor_project(ResultMessage* res_msg);

void pick_entity(int entity);
int check_if_cursor_on_entity(int entity);
int get_entity_under_cursor(void);

void update_editor(void);
void update_editor_gui(void);
void update_entity_picking(void);
void update_entity_dragging(void);

void render_editor(void);
void render_main_menu_bar(void);
void render_debug_overlay(void);
void render_scene_editor(void);
void render_brain_editor(void);
void render_simulation_editor(void);
void render_entity_handles(void);
