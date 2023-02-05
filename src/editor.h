#pragma once
#include "component.h"
#include "math.h"

typedef struct PickedEntity {
    ComponentType component_type;
    int entity;
    int is_dragging;
    int dragging_handle_idx;
} PickedEntity;

typedef struct Editor {
    PickedEntity picked_entity;
    int is_playing;
    int entity_to_copy;
} Editor;

extern Editor EDITOR;

Vec2 get_cursor_scene_pos(void);

void init_editor(void);

void pick_entity(int entity);
int check_if_cursor_on_entity(int entity);
int get_entity_under_cursor(void);

void update_editor(void);
void update_editor_gui(void);
void update_entity_picking(void);
void update_entity_dragging(void);

void render_editor(void);
void render_entity_handles(void);
