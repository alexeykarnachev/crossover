#pragma once

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#define menu_item igMenuItem_Bool
#define menu_item_ptr igMenuItem_BoolPtr

extern int IG_UNIQUE_ID;

extern ImVec2 IG_VEC2_ZERO;
extern ImVec4 IG_RED_COLOR;
extern ImVec4 IG_GREEN_COLOR;
extern ImVec4 IG_YELLOW_COLOR;
extern ImVec4 IG_PRESSED_BUTTON_COLOR;

extern ImGuiWindowFlags GHOST_WINDOW_FLAGS;
extern ImGuiColorEditFlags COLOR_PICKER_FLAGS;

void ig_same_line(void);
void ig_center_next_window(void);
void ig_drag_float(
    char* label,
    float* value,
    float min_val,
    float max_val,
    float step,
    int flags
);
void ig_drag_float2(
    char* label,
    float values[2],
    float min_val,
    float max_val,
    float step,
    int flags
);
void ig_drag_int(
    char* label, int* value, int min_val, int max_val, int step, int flags
);
void ig_add_button(const char* label, int* val, int add, int max_val);
void ig_sub_button(const char* label, int* val, int sub, int min_val);
void ig_set_button(const char* label, int* val, int set);
void ig_mem_reset_button(const char* label, void* ptr, int size);

void render_component_checkboxes(uint64_t* components);
int render_component_type_picker(
    const char* combo_name,
    int picked_type,
    int* types,
    int n_types,
    const char* type_names[]
);
