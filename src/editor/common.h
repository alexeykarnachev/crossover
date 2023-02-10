#pragma once

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#define menu_item igMenuItem_Bool
#define menu_item_ptr igMenuItem_BoolPtr

extern ImVec2 IM_VEC2_ZERO;
extern ImVec4 IM_RED_COLOR;
extern ImVec4 IM_GREEN_COLOR;
extern ImVec4 IM_YELLOW_COLOR;
extern ImVec4 IM_PRESSED_BUTTON_COLOR;

extern ImGuiWindowFlags GHOST_WINDOW_FLAGS;
extern ImGuiColorEditFlags COLOR_PICKER_FLAGS;

void same_line(void);
void center_next_window(void);
void drag_float(
    char* label,
    float* value,
    float min_val,
    float max_val,
    float step,
    int flags
);
void drag_float2(
    char* label,
    float values[2],
    float min_val,
    float max_val,
    float step,
    int flags
);
void drag_int(
    char* label, int* value, int min_val, int max_val, int step, int flags
);
void render_component_checkboxes(uint64_t* components);
